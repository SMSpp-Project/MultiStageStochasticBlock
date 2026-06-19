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

 // First, generate the objectives of all inner TwoStageStochasticBlock; each
 // of them scales its own leaves by the inner scenario probabilities.
 for( auto * block : v_Block )
  if( block ) block->generate_objective( objc );

 // Scale each inner Block's objective by its outer-stage probability, so that
 // every leaf ends up weighted by its joint probability. If no probabilities
 // are available, the inner objectives are summed unscaled. The recursive
 // scaler is inherited from TwoStageStochasticBlock.
 if( ! f_sub_block_probabilities.empty() )
  for( Index l = 0 ; l < v_Block.size() ; ++l )
   if( v_Block[ l ] )
    scale_objective_recursive( v_Block[ l ] , f_sub_block_probabilities[ l ] );

 // Now call base class to aggregate the (already scaled) objectives
 Block::generate_objective( objc );

 set_objective_generated();

} // end( MultiStageStochasticBlock::generate_objective )

/*--------------------------------------------------------------------------*/
/*------- METHODS FOR PRINTING & SAVING THE MultiStageStochasticBlock -------*/
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
 for( Index i = 0 ; i < get_number_scenarios() ; ++i ) {
  auto sub_group = group.addGroup( "Block_" + std::to_string( i ) );
  get_sub_Block( i )->serialize( sub_group );
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
