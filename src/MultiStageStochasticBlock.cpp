/*--------------------------------------------------------------------------*/
/*---------------- File MultiStageStochasticBlock.cpp ----------------------*/
/*--------------------------------------------------------------------------*/
/** @file
 * Implementation of the MultiStageStochasticBlock class.
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
/*---------------------------- IMPLEMENTATION ------------------------------*/
/*--------------------------------------------------------------------------*/
/*------------------------------ INCLUDES ----------------------------------*/
/*--------------------------------------------------------------------------*/

#include "MultiStageStochasticBlock.h"

/*--------------------------------------------------------------------------*/
/*------------------------- NAMESPACE AND USING ----------------------------*/
/*--------------------------------------------------------------------------*/

using namespace SMSpp_di_unipi_it;

/*--------------------------------------------------------------------------*/
/*----------------------------- STATIC MEMBERS -----------------------------*/
/*--------------------------------------------------------------------------*/

// register MultiStageStochasticBlock to the Block factory
SMSpp_insert_in_factory_cpp_1( MultiStageStochasticBlock );

/*--------------------------------------------------------------------------*/
/*------------------ METHODS of MultiStageStochasticBlock ------------------*/
/*--------------------------------------------------------------------------*/

void MultiStageStochasticBlock::generate_objective( Configuration * objc )
{
 if( objective_generated() ) // objective has already been generated
  return;                   // nothing to do

 // the objectives of all inner TwoStageStochasticBlock, summed: the outer and
 // inner probabilities are the weights of their Objective [see deserialize()]
 for( auto * block : v_Block )
  if( block ) block->generate_objective( objc );

 Block::generate_objective( objc );

 set_objective_generated();

} // end( MultiStageStochasticBlock::generate_objective )

/*--------------------------------------------------------------------------*/
/*------- METHODS FOR PRINTING & SAVING THE MultiStageStochasticBlock ------*/
/*--------------------------------------------------------------------------*/

void MultiStageStochasticBlock::print( std::ostream & output , char vlvl ) const
{
 output << std::endl << "MultiStageStochasticBlock with ";

 if( v_Block.empty() )
  output << "no inner Block";
 else
  output << v_Block.size() << " sub-Block" << std::endl;
}  // end( MultiStageStochasticBlock::print )

/*--------------------------------------------------------------------------*/

void MultiStageStochasticBlock::serialize( netCDF::NcGroup & group ) const
{
 Block::serialize( group );

 // type
 group.putAtt( "type" , "MultiStageStochasticBlock" );

 // NumberSubBlocks
 group.addDim( "NumberSubBlocks" , get_number_scenarios() );

 // inner TwoStageStochasticBlock, one per outer scenario
 // the weight of the Objective of each inner Block is the one it has read
 // times its outer-stage probability [see deserialize()], which is written
 // apart: the inner Block is written with the weight it has read
 for( Index i = 0 ; i < get_number_scenarios() ; ++i ) {
  auto sub_group = group.addGroup( "Block_" + std::to_string( i ) );
  auto sub = get_sub_Block( i );
  sub->serialize( sub_group );
  if( ( i < f_sub_block_probabilities.size() ) &&
      ( f_sub_block_probabilities[ i ] != 1 ) ) {
   const double w = sub->get_objective_weight() /
                    f_sub_block_probabilities[ i ];
   if( w == 1 )
    nc_del_att( sub_group.getId() , NC_GLOBAL , "ObjectiveWeight" );
   else
    sub_group.putAtt( "ObjectiveWeight" , netCDF::NcDouble() , w );
   }
 }

 // shared scenario tree, if the shared-tree input format is in use
 if( f_scenario_tree ) {
  auto sg_group = group.addGroup( "ScenarioGenerator" );
  f_scenario_tree->serialize( sg_group );
 }

 // outer-stage scenario probabilities (optional)
 if( ! f_sub_block_probabilities.empty() )
  SMSpp_di_unipi_it::serialize< double >( group , f_sub_block_probabilities ,
                                          "SubBlockProbabilities" ,
                                          "NumberSubBlocks" );

 // AbstractPath(s) to the first-stage static here-and-now variables
 if( ! v_paths_to_static_vars.empty() ) {
  auto static_path_group = group.addGroup( "StaticAbstractPath" );
  AbstractPath::serialize( v_paths_to_static_vars , static_path_group );
 }

}  // end( MultiStageStochasticBlock::serialize )

/*--------------------------------------------------------------------------*/
/*-------------- End File MultiStageStochasticBlock.cpp --------------------*/
/*--------------------------------------------------------------------------*/
