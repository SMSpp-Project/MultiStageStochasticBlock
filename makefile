##############################################################################
################################ makefile ####################################
##############################################################################
#                                                                            #
#   makefile of MultiStageStochasticBlock                                    #
#                                                                            #
#   Note that $(SMS++INC) is assumed to include any -I directive             #
#   corresponding to external libraries needed by SMS++, at least to the     #
#   extent in which they are needed by the parts of SMS++ used by            #
#   MultiStageStochasticBlock.                                               #
#                                                                            #
#   Input:  $(CC)          = compiler command                                #
#           $(SW)          = compiler options                                #
#           $(SMS++INC)    = the -I$( core SMS++ directory )                 #
#           $(SMS++OBJ)    = the libSMS++ library itself                     #
#           $(TSSBkH)      = the .h files to include for                     #
#                            TwoStageStochasticBlock                         #
#           $(TSSBkINC)    = the -I$( TwoStageStochasticBlock src dir )      #
#           $(MSSBkSDR)    = the directory where the source is               #
#                                                                            #
#   Output: $(MSSBkOBJ)    = the final object(s) / library                   #
#           $(MSSBkH)      = the .h files to include                         #
#           $(MSSBkINC)    = the -I$( source directory )                     #
#                                                                            #
#                              Antonio Frangioni                             #
#                         Dipartimento di Informatica                        #
#                             Universita' di Pisa                            #
#                                                                            #
##############################################################################


# macros to be exported - - - - - - - - - - - - - - - - - - - - - - - - - - -

MSSBkOBJ = $(MSSBkSDR)/obj/MultiStageStochasticBlock.o

MSSBkINC = -I$(MSSBkSDR)/include

MSSBkH   = $(MSSBkSDR)/include/MultiStageStochasticBlock.h

# clean - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean::
	rm -f $(MSSBkOBJ) $(MSSBkSDR)/*~

# dependencies: every .o from its .cpp + every recursively included .h- - - -

$(MSSBkSDR)/obj/MultiStageStochasticBlock.o: \
	$(MSSBkSDR)/src/MultiStageStochasticBlock.cpp \
	$(MSSBkSDR)/include/MultiStageStochasticBlock.h $(TSSBkH) $(SMS++OBJ)
	$(CC) -c $(MSSBkSDR)/src/MultiStageStochasticBlock.cpp -o $@ \
	$(MSSBkINC) $(TSSBkINC) $(StcBlkINC) $(SMS++INC) $(SW)

########################## End of makefile ###################################
