 // FakeReceiver.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/08/03
// Copyright (C) 2007-12 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************



// NO FUNCTION YET
#include "../include/FakeReceiver.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"
#include "smile.h"

namespace ssi {

char FakeReceiver::ssi_log_name[] = "faker_____";

  void CreateNetwork(void);
  void InfereceWithBayesNet(void);
  void UpgradeToInfluenceDiagram(void);
  void InferenceWithInfluenceDiagram(void);
  void ComputeValueOfInformation(void); 


FakeReceiver::FakeReceiver (const ssi_char_t *file)
	:	_file (0),
		_listener (0)
		{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);	


	 CreateNetwork();  
     InfereceWithBayesNet();  
     /*  UpgradeToInfluenceDiagram();  
     InferenceWithInfluenceDiagram();  
     ComputeValueOfInformation();  
   */
}


void CreateNetwork(void) {
    DSL_network theNet;
    
    // create node "Success"
    int success = theNet.AddNode(DSL_CPT,"Success");
    
    // setting number (and name) of outcomes
    DSL_idArray someNames;
    someNames.Add("Success");
    someNames.Add("Failure");
    theNet.GetNode(success)->Definition()->SetNumberOfOutcomes(someNames);  
 
    // create node "Forecast"
    int forecast = theNet.AddNode(DSL_CPT,"Forecast");
 
    // setting number (and name) of outcomes
    someNames.Flush();
    someNames.Add("Good");
    someNames.Add("Moderate");
    someNames.Add("Poor");
    theNet.GetNode(forecast)->Definition()->SetNumberOfOutcomes(someNames);  
    
     // add arc from "Success" to "Forecast"
     theNet.AddArc(success,forecast);
    
    // now fill in the conditional distribution for node "Success" using
    // direct method. The probabilities are:
    // P("Success" = Success) = 0.2
    // P("Success" = Failure) = 0.8 
    DSL_doubleArray theProbs;
    theProbs.SetSize(2); // it has to be an array
    theProbs[0] = 0.2;
    theProbs[1] = 0.8;
    theNet.GetNode(success)->Definition()->SetDefinition(theProbs);
 
    // now fill in the conditional distribution for node "Forecast" using a system of 
    // coordinates. The probabilities are:
    // P("Forecast" = Good | "Success" = Success) = 0.4
    // P("Forecast" = Moderate | "Success" = Success) = 0.4
    // P("Forecast" = Poor | "Success" = Success) = 0.2
    // P("Forecast" = Good | "Success" = Failure) = 0.1
    // P("Forecast" = Moderate | "Success" = Failure) = 0.3
    // P("Forecast" = Poor | "Success" = Failure) = 0.6
    DSL_sysCoordinates theCoordinates (*theNet.GetNode(forecast)->Definition());
    theCoordinates.UncheckedValue() = 0.4;
    theCoordinates.Next();
    theCoordinates.UncheckedValue() = 0.4;
    theCoordinates.Next();
    theCoordinates.UncheckedValue() = 0.2;
    theCoordinates.Next();
    theCoordinates.UncheckedValue() = 0.1;
    theCoordinates.Next();
    theCoordinates.UncheckedValue() = 0.3;
    theCoordinates.Next();
    theCoordinates.UncheckedValue() = 0.6; 
    theNet.WriteFile("tutorial.dsl");
  };


 void InfereceWithBayesNet(void) {
    DSL_network theNet;
    theNet.ReadFile("tutorial.dsl");
    
    // use clustering algorithm
   theNet.SetDefaultBNAlgorithm(DSL_ALG_BN_LAURITZEN);
 
    
    /* say that we want to compute P("Forecast" = Moderate) */
    // update the network
    theNet.UpdateBeliefs();
    
    // get the handle of node "Forecast"
    int forecast = theNet.FindNode("Forecast");
 
    // get the result value
    DSL_sysCoordinates theCoordinates(*theNet.GetNode(forecast)->Value());
    DSL_idArray *theNames;
    theNames = theNet.GetNode(forecast)->Definition()->GetOutcomesNames();  
    int moderateIndex = theNames->FindPosition("Moderate"); // should be 1
    theCoordinates[0] = moderateIndex;
    theCoordinates.GoToCurrentPosition();
    
    // get P("Forecast" = Moderate)
    double P_ForecastIsModerate = theCoordinates.UncheckedValue();
    printf("P(\"Forecast\" = Moderate) = %f\n",P_ForecastIsModerate);
    
    
    /* now we want to compute P("Success" = Failure | "Forecast" = Good) */
    // first, introduce the evidence
    // 0 is the index of state [Good]
    theNet.GetNode(forecast)->Value()->SetEvidence(0);
 
    // update the network again
    theNet.UpdateBeliefs();
    
    // get the handle of node "Success"
    int success = theNet.FindNode("Success");
    
    // get the result value
    theCoordinates.LinkTo(*theNet.GetNode(success)->Value());
    theCoordinates[0] = 1; // 1 is the index of state [Failure]
    theCoordinates.GoToCurrentPosition();
   double P_SuccIsFailGivenForeIsGood = theCoordinates.UncheckedValue();
    printf("P(\"Success\" = Failure | \"Forecast\" = Good) = ");
    printf("%f\n",P_SuccIsFailGivenForeIsGood);
    
 
    /* now we want to compute P("Success" = Success | "Forecast" = Poor) */
    // first, clear the evidence in node "Forecast"
    theNet.GetNode(forecast)->Value()->ClearEvidence();
    
    // introduce the evidence in node "Success"
    // 2 is the index of state [Poor]
    theNet.GetNode(forecast)->Value()->SetEvidence(2);
    
    // update the network again
    theNet.UpdateBeliefs();
    
    // get the result value
    theCoordinates.LinkTo(*theNet.GetNode(success)->Value());
    theCoordinates[0] = 0; // 0 is the index of state [Success]
    theCoordinates.GoToCurrentPosition();
    double P_SuccIsSuccGivenForeIsPoor = theCoordinates.UncheckedValue();
    printf("P(\"Success\" = Success | \"Forecast\" = Poor) = ");
    printf("%f\n", P_SuccIsSuccGivenForeIsPoor);
  };


 void UpgradeToInfluenceDiagram(void) {
    DSL_network theNet;
    theNet.ReadFile("tutorial.xdsl");
    
    // create decision node "Invest"
    int invest = theNet.AddNode(DSL_LIST,"Invest");
    
    // setting number (and name) of choices
    DSL_stringArray someNames;
    someNames.Add("Invest");
    someNames.Add("DoNotInvest");
    theNet.GetNode(invest)->Definition()->SetNumberOfOutcomes(someNames);
    
    // create value node "Gain"
    int gain = theNet.AddNode(DSL_TABLE,"Gain");
    
    // add arc from "Invest" to "Gain"
    theNet.AddArc(invest,gain);
    
    // add arc from "Success" to "Gain"
    int success = theNet.FindNode("Success");
    theNet.AddArc(success,gain);
    
    // now fill in the utilities for the node "Gain"
    // The utilities are:
    // U("Invest" = Invest, "Success" = Success) = 10000
    // U("Invest" = Invest, "Success" = Failure) = -5000
    // U("Invest" = DoNotInvest, "Success" = Success) = 500
    // U("Invest" = DoNotInvest, "Success" = Failure) = 500
    // get the internal matrix of the definition of node "Gain"
    DSL_Dmatrix *theMatrix;
    theNet.GetNode(gain)->Definition()->GetDefinition(&theMatrix);
    
    // and set the values directly
    theMatrix->Subscript(0) = 10000;
    theMatrix->Subscript(1) = -5000;
    theMatrix->Subscript(2) = 500;
    theMatrix->Subscript(3) = 500;
    theNet.WriteFile("tutorial.xdsl");
  };



 void InferenceWithInfluenceDiagram(void) {
    DSL_network theNet;
    theNet.ReadFile("tutorial.xdsl");
    
    // update the network
    theNet.UpdateBeliefs();
    
    // get the resulting value
    int gain = theNet.FindNode("Gain");
    DSL_sysCoordinates theCoordinates(*theNet.GetNode(gain)->Value());
    printf("\nThese are the expected utilities:\n\n");
    DSL_idArray *theOutcomesNames;
    DSL_intArray theIndexingParents;
    DSL_nodeDefinition *theDefinition;
    char *hisOutcomeName;
    const char *hisName;
    double ExpectedUtility;
    int aParent;
    int hisOutcome;
    int x;
    int result = DSL_OKAY;
    theIndexingParents = theNet.GetNode(gain)->Value()->GetIndexingParents();
    theCoordinates.GoFirst(); // goes to (0,0)
    
    /* print the expected utility of each choice */
    while (result != DSL_OUT_OF_RANGE) {
      printf("Policy:\n");
      for (x=0; x<theIndexingParents.NumItems(); x++) {
        aParent = theIndexingParents[x];
        theDefinition = theNet.GetNode(aParent)->Definition();
        theOutcomesNames = theDefinition->GetOutcomesNames();
    
        // get the name of the current outcome for this node
        // the current outcome is in the coordinates
        hisOutcome = theCoordinates[x];
        hisOutcomeName = (*theOutcomesNames)[hisOutcome];
        hisName = theNet.GetNode(aParent)->Info().Header().GetId();
        printf(" Node \"%s\" = %s\n",hisName,hisOutcomeName);
      };
      ExpectedUtility = theCoordinates.UncheckedValue();
      printf(" Expected Utility = %f\n\n",ExpectedUtility);
      result = theCoordinates.Next();
    };
  };


	 void ComputeValueOfInformation(void) {
    DSL_network theNet;
    theNet.ReadFile("tutorial.xdsl");
    DSL_valueOfInformation theValue(&theNet);
    
    // get the handle of nodes "Forecast" and "Invest"
    int forecast = theNet.FindNode("Forecast");
    int invest = theNet.FindNode("Invest");
   theValue.AddNode(forecast);
    theValue.SetDecision(invest);
    theNet.ValueOfInformation(theValue);
    DSL_Dmatrix &theResult = theValue.GetValues();
   double EVIForecast = theResult[0];
    printf("Expected Value of Information(\"Forecast\") = %f\n",EVIForecast);
    theNet.WriteFile("tutorial.xdsl");
  };
FakeReceiver::~FakeReceiver () {
	
	ssi_event_destroy (_event);

}

bool FakeReceiver::setEventListener (IEventListener *listener) {

	_listener = listener;
	_event.sender_id = _listener->getStringId (_options.sname);
	if (_event.sender_id == SSI_EVENT_STRINGID_UNDEF) {
		return false;
	}
	_event.event_id = _listener->getStringId (_options.ename);
	if (_event.event_id == SSI_EVENT_STRINGID_UNDEF) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	return true;

}

void FakeReceiver::enter (ITheEventBoard &board){

	if (_listener) {
		ssi_event_adjust (_event, 1 * sizeof (ssi_event_map_t));
	}

}

void FakeReceiver::update (ITheEventBoard &board, IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	for(ssi_size_t i = 0; i < n_new_events; i++){
		ssi_print("\nreceiver alive");
	}

	/*ssi_event_t *e = 0;
	events.reset ();
	for(ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
		ssi_char_t string[SSI_MAX_CHAR];
		e = events.next();
		if (_listener) {
			_event.dur = time_ms - _event.time;
			_event.time = time_ms;		
			ssi_event_map_t *ptr = ssi_pcast (ssi_event_map_t, _event.ptr);
			ssi_event_map_t *in_ptr = ssi_pcast (ssi_event_map_t, e->ptr);
			
			for(ssi_size_t ndim = 0; ndim < _dim; ndim++){
				ssi_sprint(string, "Dimension%u", ndim);
				ptr[ndim].id = board.AddString(string);
				if(_options.mapped){
					ptr[ndim].value = _mapping[ndim];
				}else{
					if(in_ptr->value >= -1.0f && in_ptr->value <= 1.0f){
						ptr[ndim].value = in_ptr->value * _mapping[ndim]; 
					}
					else{
						ptr[ndim].value = 0.0f;
					}
				}
			}

			_listener->update (_event);
			e = 0;
		}
	}*/

}

void FakeReceiver::flush (ITheEventBoard &board){

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	if (_listener) {
		ssi_event_reset (_event);
	}

}

}