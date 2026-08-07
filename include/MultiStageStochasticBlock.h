/*--------------------------------------------------------------------------*/
/*----------------- File MultiStageStochasticBlock.h -----------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Header file of MultiStageStochasticBlock, a class for representing a
 * multi-stage stochastic programming problem as an aggregation of
 * TwoStageStochasticBlock.
 *
 * \author Antonio Frangioni \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \author Donato Meoli \n
 *         Dipartimento di Informatica \n
 *         Universita' di Pisa \n
 *
 * \copyright &copy; by Antonio Frangioni and Donato Meoli
 */
/*--------------------------------------------------------------------------*/
/*----------------------------- DEFINITIONS --------------------------------*/
/*--------------------------------------------------------------------------*/

#ifndef __MultiStageStochasticBlock
#define __MultiStageStochasticBlock
                      /* self-identification: #endif at the end of the file */

/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "TwoStageStochasticBlock.h"

// MSSB consumes only the abstract MultiStageScenarioGenerator (declared in
// ScenarioGenerator.h, pulled in via TwoStageStochasticBlock.h), not any
// concrete generator: the shared scenario tree is read through the generic
// View interface (root_view() / View::descend()).

/*--------------------------------------------------------------------------*/
/*----------------------------- NAMESPACE ----------------------------------*/
/*--------------------------------------------------------------------------*/

/// namespace for the Structured Modeling System++ (SMS++)
namespace SMSpp_di_unipi_it {

/*--------------------------------------------------------------------------*/
/*------------------ CLASS MultiStageStochasticBlock -----------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------- GENERAL NOTES --------------------------------*/
/*--------------------------------------------------------------------------*/
/// MultiStageStochasticBlock, a multi-stage stochastic problem
/** The MultiStageStochasticBlock represents a multi-stage stochastic
 * programming problem built as an aggregation of TwoStageStochasticBlock, in
 * the same way a multi-stage scenario tree is obtained by recursively nesting
 * two-stage subtrees.
 *
 * The class derives from TwoStageStochasticBlock and reuses all its machinery:
 * its inner sub-Blocks (v_Block) are one TwoStageStochasticBlock per
 * outer-stage scenario, and the inherited code that builds the
 * non-anticipativity constraints, scales the objectives and reads/writes the
 * Solution works unchanged once two small things are specialized:
 *
 * - the here-and-now AbstractPath are resolved one level deeper, against the
 *   representative scenario sub-Block of each inner TwoStageStochasticBlock,
 *   where the first-stage variables physically live (overriding
 *   get_first_stage_block());
 *
 * - the per-sub-Block objective weights are the outer-stage scenario
 *   probabilities (overriding generate_objective()).
 *
 * Each inner TwoStageStochasticBlock already enforces non-anticipativity over
 * its own here-and-now variables; the MultiStageStochasticBlock adds an outer
 * layer of non-anticipativity that ties the first-stage (root) here-and-now
 * variables across all of them. Because the inherited
 * TwoStageStochasticBlockSolution recursively saves the Solution of each
 * sub-Block (here a TwoStageStochasticBlockSolution in turn), the whole tree
 * is serialized, solved and read back with no dedicated Solution class.
 *
 * For a three-stage problem the first stage holds the common here-and-now
 * decisions, the second stage holds one TwoStageStochasticBlock per outer
 * scenario, and the third stage holds the recourse decisions in the leaves of
 * each inner two-stage subtree. Deeper trees are obtained by letting the inner
 * sub-Blocks be themselves MultiStageStochasticBlock. */

 class MultiStageStochasticBlock : public TwoStageStochasticBlock {

/*--------------------------------------------------------------------------*/
/*----------------------- PUBLIC PART OF THE CLASS -------------------------*/
/*--------------------------------------------------------------------------*/

 public:

/*--------------------------------------------------------------------------*/
/*-------- CONSTRUCTING AND DESTRUCTING MultiStageStochasticBlock ----------*/
/*--------------------------------------------------------------------------*/
 /** @name Constructing and destructing MultiStageStochasticBlock
  *  @{ */

 /// constructor: takes the \p father Block (default nullptr, void ctor)

 explicit MultiStageStochasticBlock( Block * father = nullptr )
  : TwoStageStochasticBlock( father ) {}

/*--------------------------------------------------------------------------*/
 /// destructor: releases the shared scenario tree, if any
 /** The per-inner-Block views into the tree are owned (and deleted) by the
  * inner TwoStageStochasticBlock themselves; they never dereference the tree
  * at destruction, so releasing it here is safe regardless of order. */

 virtual ~MultiStageStochasticBlock() { delete f_scenario_tree; }

/*--------------------------------------------------------------------------*/
 /// de-serialize a MultiStageStochasticBlock out of netCDF::NcGroup
 /** The method de-serializes a MultiStageStochasticBlock. Besides the
  * mandatory "type" attribute of any :Block, the group must contain:
  *
  * - NumberSubBlocks: dimension with the number L of inner
  *   TwoStageStochasticBlock (one per outer-stage scenario);
  *
  * - Block_0, ..., Block_{L-1}: groups, each a TwoStageStochasticBlock in
  *   factory format;
  *
  * - SubBlockProbabilities (optional): a netCDF::NcDouble variable indexed
  *   over NumberSubBlocks with the outer-stage scenario probabilities; if
  *   absent the inner objectives are summed unscaled;
  *
  * - StaticAbstractPath (optional): the AbstractPath to the first-stage
  *   (here-and-now) static variables, given relative to the representative
  *   scenario sub-Block of each inner TwoStageStochasticBlock;
  *
  * - DynamicAbstractPath (optional): currently not supported.
  *
  * @param group A netCDF::NcGroup holding the data describing this
  *              MultiStageStochasticBlock.
  */

 void deserialize( const netCDF::NcGroup & group ) override {
  Index n_sub_blocks = 0;
  deserialize_dim( group , "NumberSubBlocks" , n_sub_blocks , false );

  if( n_sub_blocks == 0 )
   throw(std::invalid_argument( "MultiStageStochasticBlock::deserialize: "
    "'NumberSubBlocks' must be positive." ) );

  f_number_scenarios = n_sub_blocks;

  v_Block.reserve( f_number_scenarios );

  // Two input formats are supported, chosen by the presence of a single
  // "ScenarioGenerator" subgroup:
  //
  // - SHARED-TREE (preferred): the subgroup holds the whole scenario tree (a
  //   MultiStageDiscreteScenarioSet). The outer-stage scenarios are the
  //   children of its root, their probabilities are the outer-stage weights,
  //   and each inner Block is handed a read-only view onto its own outer node
  //   (before deserialization) from which it draws its inner scenarios; the
  //   inner Block_l groups carry only the model template (no per-Block
  //   DiscreteScenarioSet).
  //
  // - BAKED (legacy): each inner Block_l is a self-contained
  //   TwoStageStochasticBlock with its own DiscreteScenarioSet, and the
  //   outer-stage weights are read from "SubBlockProbabilities".
  auto sg_group = group.getGroup( "ScenarioGenerator" );

  if( ! sg_group.isNull() ) {
   auto * sg = ScenarioGenerator::new_ScenarioGenerator( sg_group );
   f_scenario_tree = dynamic_cast< MultiStageScenarioGenerator * >( sg );
   if( ! f_scenario_tree ) {
    delete sg;
    throw(std::logic_error( "MultiStageStochasticBlock::deserialize: the "
     "'ScenarioGenerator' is not a MultiStageScenarioGenerator." ) );
   }

   // a View pinned at the root: its pool are the outer-stage (first-stage)
   // realizations; a clone of it moved down with descend() is the View
   // pinned at the outer scenario currently selected, whose pool are that
   // scenario's inner realizations. The MSSB consumes only this general
   // MultiStageScenarioGenerator interface, not the concrete type.
   auto root = f_scenario_tree->root_view();
   const Index L = root->get_support_size();
   if( L != f_number_scenarios )
    throw(std::invalid_argument(
     "MultiStageStochasticBlock::deserialize: the scenario tree root has " +
     std::to_string( L ) + " children but NumberSubBlocks is " +
     std::to_string( f_number_scenarios ) + "." ) );

   f_sub_block_probabilities.resize( f_number_scenarios );

   // build each inner Block, handing it the View onto its outer node *before*
   // deserialization so it draws its scenarios from the shared tree
   for( Index l = 0 ; l < f_number_scenarios ; ++l ) {
    f_sub_block_probabilities[ l ] =                     // P(s_l)
     root->get_current_scenario_probability();

    auto sub_group = group.getGroup( "Block_" + std::to_string( l ) );
    if( sub_group.isNull() )
     throw(std::logic_error( "MultiStageStochasticBlock::deserialize: group "
      "'Block_" + std::to_string( l ) + "' not found." ) );

    auto type_att = sub_group.getAtt( "type" );
    if( type_att.isNull() )
     throw(std::logic_error( "MultiStageStochasticBlock::deserialize: "
      "'Block_" + std::to_string( l ) + "' has no 'type' attribute." ) );
    std::string type;
    type_att.getValues( type );

    auto * tssb = dynamic_cast< TwoStageStochasticBlock * >(
                                       new_Block( type , this ) );
    if( ! tssb )
     throw(std::logic_error( "MultiStageStochasticBlock::deserialize: "
      "'Block_" + std::to_string( l ) +
      "' is not a TwoStageStochasticBlock." ) );

    // the inner Block gets a View of its own, so that the root one stays
    // where it is and the L of them are independent of one another
    auto inner_view = root->clone();
    if( ! inner_view->descend() )
     throw(std::logic_error( "MultiStageStochasticBlock::deserialize: outer "
      "node " + std::to_string( l ) +
      " has no inner stage (it is a leaf)." ) );
    tssb->set_scenario_generator( inner_view.release() );
    tssb->deserialize( sub_group );
    v_Block.push_back( tssb );

    if( l + 1 < f_number_scenarios )
     root->next_scenario();
   }
  }
  else {
   // de-serialize the inner TwoStageStochasticBlock, one per outer scenario
   for( Index i = 0 ; i < f_number_scenarios ; ++i ) {
    auto sub_group = group.getGroup( "Block_" + std::to_string( i ) );
    if( sub_group.isNull() )
     throw(std::logic_error( "MultiStageStochasticBlock::deserialize: group "
      "'Block_" + std::to_string( i ) + "' not found." ) );

    auto * sub_block = new_Block( sub_group , this );
    if( ! dynamic_cast< TwoStageStochasticBlock * >( sub_block ) )
     throw(std::logic_error( "MultiStageStochasticBlock::deserialize: "
      "sub-Block 'Block_" + std::to_string( i ) +
      "' is not a TwoStageStochasticBlock." ) );

    v_Block.push_back( sub_block );
   }

   // outer-stage scenario probabilities (optional)
   SMSpp_di_unipi_it::deserialize< double >(
    group , "SubBlockProbabilities" , f_sub_block_probabilities , true );
   if( ( ! f_sub_block_probabilities.empty() ) &&
       ( f_sub_block_probabilities.size() != f_number_scenarios ) )
    throw(std::invalid_argument( "MultiStageStochasticBlock::deserialize: "
     "'SubBlockProbabilities' has the wrong size." ) );
  }

  // AbstractPath(s) to the first-stage static here-and-now variables; the
  // group is optional, as an MSSB tying no first-stage variables across its
  // inner Blocks (e.g., a single inner Block) is well-defined, in which case
  // the outer non-anticipativity constraints are simply vacuous.
  auto static_path_group = group.getGroup( "StaticAbstractPath" );
  if( ! static_path_group.isNull() )
   AbstractPath::vector_deserialize( static_path_group ,
                                     v_paths_to_static_vars );

  auto dynamic_path_group = group.getGroup( "DynamicAbstractPath" );
  if( ! dynamic_path_group.isNull() )
   throw(std::invalid_argument( "MultiStageStochasticBlock::deserialize: "
    "cannot handle dynamic here-and-now variables right now." ) );

  Block::deserialize( group );
 }

/** @} ---------------------------------------------------------------------*/
/*-------- METHODS FOR Saving / READING THE DATA ---------------------------*/
/*--------------------------------------------------------------------------*/
 /** @name Saving and reading the data of the MultiStageStochasticBlock
  *  @{ */

 void print( std::ostream & output , char vlvl = 0 ) const override;

 /// serialize a MultiStageStochasticBlock into a netCDF::NcGroup
 /** Serialize a MultiStageStochasticBlock into a netCDF::NcGroup with the
  * format explained in the comments of the deserialize() method. */

 void serialize( netCDF::NcGroup & group ) const override;

/*--------------------------------------------------------------------------*/

 /// returns the Block against which the here-and-now paths are resolved
 /** Overrides TwoStageStochasticBlock::get_first_stage_block() to descend to
  * the representative leaf Block of the inner subtree, where the first-stage
  * variables physically live. The descent is recursive through the virtual
  * itself, so it works for arbitrarily deep nesting (a
  * MultiStageStochasticBlock whose sub-Blocks are themselves
  * MultiStageStochasticBlock): if the inner Block is a
  * (Two/Multi)StageStochasticBlock, its own get_first_stage_block(0) is
  * followed; otherwise the inner Block (or its first sub-Block) is returned. */

 Block * get_first_stage_block( Index scenario ) const override {
  auto inner = get_sub_Block( scenario );
  if( auto tssb = dynamic_cast< TwoStageStochasticBlock * >( inner ) )
   return( tssb->get_first_stage_block( 0 ) );
  const auto & nested = inner->get_nested_Blocks();
  return( nested.empty() ? inner : nested.front() );
  }

/*--------------------------------------------------------------------------*/

 /// returns the number of inner TwoStageStochasticBlock (outer-stage scenarios)

 Index get_number_sub_blocks( void ) const { return( get_number_scenarios() ); }

/*--------------------------------------------------------------------------*/

 /// returns the outer-stage scenario probabilities (empty == equal weights)

 const std::vector< double > & get_sub_block_probabilities( void ) const {
  return( f_sub_block_probabilities );
  }

/** @} ---------------------------------------------------------------------*/
/*-------------------- OTHER INITIALIZATIONS -------------------------------*/
/*--------------------------------------------------------------------------*/
 /** @name Other initializations
  *  @{ */

 /// generate the objective of the MultiStageStochasticBlock
 /** Generates the objectives of all inner TwoStageStochasticBlock (which scale
  * their own leaves by the inner scenario probabilities), then scales each
  * inner Block's objective by its outer-stage scenario probability, so that
  * every leaf ends up weighted by its joint probability, and finally lets the
  * base class aggregate them. If no outer-stage probabilities are available,
  * the inner objectives are summed unscaled. */

 void generate_objective( Configuration * objc = nullptr ) override;

/** @} ---------------------------------------------------------------------*/
/*--------------------- PROTECTED PART OF THE CLASS ------------------------*/
/*--------------------------------------------------------------------------*/

 protected:

/*---------------------------- PROTECTED FIELDS  ---------------------------*/

 std::vector< double > f_sub_block_probabilities;
 ///< The outer-stage scenario probabilities (empty == implicit equal weights)

 MultiStageScenarioGenerator * f_scenario_tree = nullptr;
 ///< Shared scenario tree; non-null only for the shared-tree input format

/*--------------------------------------------------------------------------*/

 SMSpp_insert_in_factory_h;

/*--------------------------------------------------------------------------*/

 }; // end( class MultiStageStochasticBlock )

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

}  // end( namespace SMSpp_di_unipi_it )

/*--------------------------------------------------------------------------*/

#endif /* MultiStageStochasticBlock.h included */

/*--------------------------------------------------------------------------*/
/*--------------- End File MultiStageStochasticBlock.h ---------------------*/
/*--------------------------------------------------------------------------*/
