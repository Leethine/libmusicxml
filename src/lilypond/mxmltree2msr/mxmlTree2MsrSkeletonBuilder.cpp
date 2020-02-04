/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include <climits>      // INT_MIN, INT_MAX
#include <regex>

#include "messagesHandling.h"

#include "msr2Summary.h"

#include "generalOah.h"

#include "setTraceOahIfDesired.h"
#ifdef TRACE_OAH
  #include "traceOah.h"
#endif

#include "mxmlTreeOah.h"
#include "msrOah.h"

#include "mxmlTree2MsrSkeletonBuilder.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_mxmlPartGroupDescr mxmlPartGroupDescr::create (
  int            startInputLineNumber,
  int            partGroupNumber,
  S_msrPartGroup partGroup,
  int            startPosition)
{
  mxmlPartGroupDescr* o = new
    mxmlPartGroupDescr (
      startInputLineNumber,
      partGroupNumber,
      partGroup,
      startPosition);
  assert(o!=0);
  return o;
}

mxmlPartGroupDescr::mxmlPartGroupDescr (
  int            startInputLineNumber,
  int            partGroupNumber,
  S_msrPartGroup partGroup,
  int            startPosition)
{
  fStartInputLineNumber   = startInputLineNumber;
  fStopInputLineNumber   = -1;

  fPartGroupNumber = partGroupNumber;
  fPartGroup       = partGroup;

  fStartPosition   = startPosition;
  fStopPosition    = -1;
}

mxmlPartGroupDescr::~mxmlPartGroupDescr ()
{}

string mxmlPartGroupDescr::partGroupDescrAsString () const
{
  stringstream s;

  s <<
    "'" <<
    fPartGroupNumber <<
    "' -=> " <<
    fPartGroup->getPartGroupCombinedName ();

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    s <<
      ", positions " <<
      fStartPosition << ".." << fStopPosition;
  }
#endif

  s <<
    ", lines " <<
    fStartInputLineNumber << ".." << fStopInputLineNumber;

  return s.str ();
}

void mxmlPartGroupDescr::print (ostream& os) const
{
  const int fieldWidth = 14;

  os << left <<
    setw (fieldWidth) <<
    "partGroupNumber" << " : " <<
    fPartGroupNumber <<
    endl <<
    setw (fieldWidth) <<
    "partGroup" << " : " <<
    fPartGroup <<
    endl <<
    setw (fieldWidth) <<
    "startPosition" << " : " <<
    fStartPosition <<
    endl <<
    setw (fieldWidth) <<
    "startInputLineNumber" << " : " <<
    fStartInputLineNumber <<
    endl <<
    "stopPosition" << " : " <<
    fStopPosition <<
    endl <<
    setw (fieldWidth) <<
    "stopInputLineNumber" << " : " <<
    fStopInputLineNumber <<
    endl;
}

ostream& operator<< (ostream& os, const S_mxmlPartGroupDescr& elt)
{
  elt->print (os);
  return os;
}

//_______________________________________________________________________________
mxmlTree2MsrSkeletonBuilder::mxmlTree2MsrSkeletonBuilder (
  indentedOstream& ios)
  : fLogOutputStream (ios)
{
  // the MSR score we're building
  fMsrScore =
    msrScore::create (0);

  // score handling
  fScoreNumberOfMeasures = 0;

  // scaling handling
  fCurrentMillimeters = -1;
  fCurrentTenths      = -1;

  fOnGoingSystemMargins = false;

  // page layout
  fOnGoingPageLayout = false;

  fOnGoingPageMargins = false;
  fCurrentMarginTypeKind = kBothMargins; // default value

  fOnGoingSystemLayout = false;

  // part groups handling
  fPartGroupsCounter = 0;
  fOnGoingPartGroupNameDisplay = false;

  // parts handling
  fCurrentNoIDPartNumber = 0;
  fCurrentPartsPosition = 0;
  fOnGoingPartNameDisplay = false;

  // staff handling
  fCurrentStaffMusicXMLNumber = -1;

  // voice handling
  fCurrentVoiceMusicXMLNumber = -1;

  // measures
  fPartNumberOfMeasures = 0;

  // lyrics handling
  fCurrentStanzaNumber = K_NO_STANZA_NUMBER; // JMI
  fCurrentStanzaName = K_NO_STANZA_NAME; // JMI

  // harmonies handling
  fThereAreHarmoniesToBeAttachedToCurrentNote = false;
  fHarmonyVoicesCounter = 0;

  // figured bass handling
  fThereAreFiguredBassToBeAttachedToCurrentNote = false;
  fFiguredBassVoicesCounter = 0;

  // ongoing note
  fOnGoingNote = false;

  // note context
  fCurrentStaffMusicXMLNumber = 0;
  fCurrentVoiceMusicXMLNumber = 0;

  // create an empty list for part groups starting at 0,
  // i.e. for the implicit part group
  fPositionStartingPartGroupDescrsVector.push_back (
    list<S_mxmlPartGroupDescr> ());

  // create an empty list for part groups stopping at 0,
  // that will remain empty
  fPositionStoppingPartGroupDescrsVector.push_back (
    list<S_mxmlPartGroupDescr> ());

  // create the implicit part group
  createImplicitPartGroup ();
}

mxmlTree2MsrSkeletonBuilder::~mxmlTree2MsrSkeletonBuilder ()
{}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::browseMxmlTree (
  const Sxmlelement& mxmlTree)
{
  if (mxmlTree) {
    // create a tree browser on this visitor
    tree_browser<xmlelement> browser (this);

    // browse the xmlelement tree
    browser.browse (*mxmlTree);
  }
}

//______________________________________________________________________________
S_mxmlPartGroupDescr mxmlTree2MsrSkeletonBuilder::fetchStartedPartGroupDescr (
  int partGroupNumber)
{
  S_mxmlPartGroupDescr result;

  if (fStartedPartGroupDescrsMap.count (partGroupNumber)) {
    result =
      fStartedPartGroupDescrsMap [partGroupNumber];
  }

  return result;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showAllPartGroupDescrsMap (
  int inputLineNumber)
{
  fLogOutputStream <<
    "AllPartGroupDescrsMap:" <<
    endl;

  if (fAllPartGroupDescrsMap.size ()) {
    gIndenter++;

    map<int, S_mxmlPartGroupDescr>::const_iterator
      iBegin = fAllPartGroupDescrsMap.begin (),
      iEnd   = fAllPartGroupDescrsMap.end (),
      i      = iBegin;

    for ( ; ; ) {
      S_mxmlPartGroupDescr
        partGroupDescr = (*i).second;

      S_msrPartGroup
        partGroup =
          partGroupDescr->getPartGroup ();

      fLogOutputStream <<
        partGroupDescr->partGroupDescrAsString () <<
        endl;

        gIndenter++;

        partGroup->
          printPartGroupParts (
            inputLineNumber,
            fLogOutputStream);

        gIndenter--;

      if (++i == iEnd) break;
      // no endl here
    } // for

    gIndenter--;
  }

  else {
    fLogOutputStream <<
      gTab << "empty map" <<
      endl;
  }

  fLogOutputStream <<
    "------------------" <<
    endl;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showStartedPartGroupDescrsMap (
  int inputLineNumber)
{
  fLogOutputStream <<
    "StartedPartGroupDescrsMap:" <<
    endl;

  if (fStartedPartGroupDescrsMap.size ()) {
    gIndenter++;

    map<int, S_mxmlPartGroupDescr>::const_iterator
      iBegin = fStartedPartGroupDescrsMap.begin (),
      iEnd   = fStartedPartGroupDescrsMap.end (),
      i      = iBegin;

    for ( ; ; ) {
      S_mxmlPartGroupDescr
        partGroupDescr = (*i).second;

      S_msrPartGroup
        partGroup =
          partGroupDescr->getPartGroup ();

      fLogOutputStream <<
        partGroupDescr->partGroupDescrAsString () <<
        endl;

        gIndenter++;

        partGroup->
          printPartGroupParts (
            inputLineNumber,
            fLogOutputStream);

        gIndenter--;

      if (++i == iEnd) break;
      // no endl here
    } // for

    gIndenter--;
  }

  else {
    fLogOutputStream <<
      gTab << "empty map" <<
      endl;
  }

  fLogOutputStream <<
    "------------------" <<
    endl;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showPartGroupsStack (
  int inputLineNumber)
{
  fLogOutputStream <<
    "PartGroupsDescrStack:" <<
    endl;

  if (fPartGroupsDescrStack.size ()) {
    gIndenter++;

    list<S_mxmlPartGroupDescr>::const_iterator
      iBegin = fPartGroupsDescrStack.begin (),
      iEnd   = fPartGroupsDescrStack.end (),
      i      = iBegin;

    for ( ; ; ) {
      fLogOutputStream <<
        "v " <<
        (*i)->partGroupDescrAsString () <<
        endl;
      if (++i == iEnd) break;
      // no endl here
    } // for

    gIndenter--;
  }

  else {
    fLogOutputStream <<
      gTab << "empty stack" <<
      endl;
  }

  fLogOutputStream <<
    "------------------" <<
    endl;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showPartGroupDescrsVector (
  int inputLineNumber)
{
  fLogOutputStream <<
    "PartGroupDescrsVector:" <<
    endl;

  gIndenter++;

  for (unsigned int i = 0; i < fPartGroupDescsVector.size (); i++) {
    S_mxmlPartGroupDescr
      partGroupDescr = fPartGroupDescsVector [i];

    S_msrPartGroup
      partGroup =
        partGroupDescr->getPartGroup ();

    fLogOutputStream <<
      i + 1 << ": " <<
        partGroupDescr->partGroupDescrAsString () <<
      ", line " << inputLineNumber <<
      endl;
  } // for

  fLogOutputStream <<
    "------------------" <<
    endl;

  gIndenter--;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showPositionStartingPartGroupDescrsVector (
  int inputLineNumber)
{
  fLogOutputStream <<
    "PositionStartingPartGroupDescrsVector:" <<
    endl;

  if (fPositionStartingPartGroupDescrsVector.size ()) {
    gIndenter++;

    for (unsigned int k = 0; k < fPositionStartingPartGroupDescrsVector.size (); k++) {
      fLogOutputStream <<
        k << ": " <<
        endl;

      list<S_mxmlPartGroupDescr>&
        startingPartGroupDescrsList =
          fPositionStartingPartGroupDescrsVector [k];

      if (startingPartGroupDescrsList.size ()) {
        gIndenter++;

        list<S_mxmlPartGroupDescr>::const_iterator
          iBegin = startingPartGroupDescrsList.begin (),
          iEnd   = startingPartGroupDescrsList.end (),
          i      = iBegin;

        for ( ; ; ) {
          S_mxmlPartGroupDescr
            partGroupDescr = (*i);

          S_msrPartGroup
            partGroup =
              partGroupDescr->getPartGroup ();

          fLogOutputStream <<
              partGroupDescr->partGroupDescrAsString () <<
            ", line " << inputLineNumber <<
            endl;

          if (++i == iEnd) break;
          // no endl here
        } // for

        gIndenter--;
      }

      else {
        fLogOutputStream <<
          gTab << "empty list" <<
          endl;
      }
    }

    gIndenter--;
  }

  fLogOutputStream <<
    "------------------" <<
    endl;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showPositionStoppingPartGroupDescrsVector (
  int inputLineNumber)
{
  fLogOutputStream <<
    "PositionStoppingPartGroupDescrsVector:" <<
    endl;

  if (fPositionStoppingPartGroupDescrsVector.size ()) {
    gIndenter++;

    for (unsigned int k = 0; k < fPositionStoppingPartGroupDescrsVector.size (); k++) {
      fLogOutputStream <<
        k << ": " <<
        endl;

      list<S_mxmlPartGroupDescr>&
        theList =
          fPositionStoppingPartGroupDescrsVector [k];

      if (theList.size ()) {
        gIndenter++;

        list<S_mxmlPartGroupDescr>::const_iterator
          iBegin = theList.begin (),
          iEnd   = theList.end (),
          i      = iBegin;

        for ( ; ; ) {
          S_mxmlPartGroupDescr
            partGroupDescr = (*i);

          S_msrPartGroup
            partGroup =
              partGroupDescr->getPartGroup ();

          fLogOutputStream <<
              partGroupDescr->partGroupDescrAsString () <<
            ", line " << inputLineNumber <<
            endl;

          if (++i == iEnd) break;
          // no endl here
        } // for

        gIndenter--;
      }

      else {
        fLogOutputStream <<
          gTab << "empty list" <<
          endl;
      }
    }

    gIndenter--;
  }

  fLogOutputStream <<
    "------------------" <<
    endl;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showPartsVector (
  int inputLineNumber)
{
  fLogOutputStream <<
    "PartsVector:" <<
    endl;

  if (fPartsVector.size ()) {
    gIndenter++;

    for (unsigned int i = 0; i < fPartsVector.size (); i++) {
      S_msrPart
        part = fPartsVector [i];

      fLogOutputStream <<
        i + 1 << ": " <<
        part->getPartCombinedName () <<
        ", upLink to: ";

      S_msrPartGroup
        partPartGroupUpLink =
          part->
            getPartPartGroupUpLink ();

      if (partPartGroupUpLink) {
        fLogOutputStream <<
          partPartGroupUpLink->
            getPartGroupCombinedName ();
      }
      else {
        fLogOutputStream <<
          "not yet established";
      }

      fLogOutputStream <<
        ", line " << inputLineNumber <<
        endl;
    } // for

    gIndenter--;
  }

  else {
    fLogOutputStream <<
      gTab << "empty vector" <<
      endl;
  }

  fLogOutputStream <<
    "------------------" <<
    endl;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::showPartGroupsData (
  int    inputLineNumber,
  string context)
{
  fLogOutputStream <<
    endl <<
    context <<
    ", fCurrentPartsPosition = " << fCurrentPartsPosition <<
    ", line " << inputLineNumber <<
    ":" <<
    endl <<
    ">>> ================================================" <<
    endl;

  showAllPartGroupDescrsMap (
    inputLineNumber);
  fLogOutputStream << endl;

  showStartedPartGroupDescrsMap (
    inputLineNumber);
  fLogOutputStream << endl;

  showPartGroupDescrsVector (
    inputLineNumber);
  fLogOutputStream << endl;

  showPositionStartingPartGroupDescrsVector (
    inputLineNumber);
  fLogOutputStream << endl;

  showPositionStoppingPartGroupDescrsVector (
    inputLineNumber);
  fLogOutputStream << endl;

  showPartsVector (
    inputLineNumber);
  fLogOutputStream << endl;

  showPartGroupsStack (
    inputLineNumber);
  fLogOutputStream << endl;

  fLogOutputStream <<
    "<<< ================================================" <<
    endl <<
    endl;
}

//________________________________________________________________________
S_mxmlPartGroupDescr mxmlTree2MsrSkeletonBuilder::fetchPartGroupDescrStackTop ()
{
  S_mxmlPartGroupDescr result;

  // the current part group is the top of the stack,
  // i.e. the front for the list used to implement it
  if (fPartGroupsDescrStack.size () != 0) {
   result =
    fPartGroupsDescrStack.front ();
  }

  return result;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::registerPartGroupDescrAsStarted (
  int                  inputLineNumber,
  S_mxmlPartGroupDescr partGroupDescr)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Registering part group descr '" <<
      partGroupDescr->getPartGroupCombinedName () <<
      " as started" <<
      ", line " << inputLineNumber <<
      endl;
  }

  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "BEFORE registering part group " +
      partGroupDescr->
        partGroupDescrAsString () +
      " in the data");
  }
#endif

  // register part group in part groups vector, i.e. by appearance order
  fPartGroupDescsVector.push_back (
    partGroupDescr);

  // register it in fAllPartGroupDescrsMap under its position as part group,
  // since its part group number may reused later by other part groups
  fAllPartGroupDescrsMap [fPartGroupsCounter] =
    partGroupDescr;

  // register it temporarily in fStartedPartGroupDescrsMap
  // under its part group number,
  int partGroupNumber =
    partGroupDescr->
      getPartGroup ()->
        getPartGroupNumber ();

  fStartedPartGroupDescrsMap [partGroupNumber] =
    partGroupDescr;

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "AFTER registering part group " +
      partGroupDescr->
        partGroupDescrAsString () +
      " in the data");
  }
#endif
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::insertPartGroupDescInStartingList (
  int                  inputLineNumber,
  S_mxmlPartGroupDescr partGroupDescr,
  list<S_mxmlPartGroupDescr>&
                       startingPartGroupDescrsList)
{
  // this list is created in stop position decreasing order,
  // so as to have the outer-most part group descr appearing first
  list<S_mxmlPartGroupDescr>::iterator
    iBegin = startingPartGroupDescrsList.begin (),
    iEnd   = startingPartGroupDescrsList.end (),
    i      = iBegin;

  while (true) {
    if (i == iEnd) {
      // append partGroupDescr to the list
#ifdef TRACE_OAH
      if (gTraceOah->fTracePartGroups) {
        fLogOutputStream <<
          "Appending part group descr " <<
          partGroupDescr->
            partGroupDescrAsString () <<
          "' at the end of the list of part group descrs starting at position " <<
          fCurrentPartsPosition <<
          ", line " << inputLineNumber <<
          endl;
      }
#endif

      startingPartGroupDescrsList.push_back (
        partGroupDescr);
      /*
      stringstream s;
      s <<
        "part group descr " <<
        partGroupDescr->partGroupDescrAsString () <<
        " could not be inserted in part groups to be stopped list";

      msrInternalError (
        gOahOah->fInputSourceName,
        inputLineNumber,
        __FILE__, __LINE__,
        s.str ());
        */
      break;
    }

    // fetch current element in the list
    S_mxmlPartGroupDescr
      currentPartGroupDescr = (*i);

    if (
      partGroupDescr->getStopPosition ()
        >
      currentPartGroupDescr->getStopPosition ()
    ) {
      // insert partGroupDescr before currentPartGroupDescr
#ifdef TRACE_OAH
      if (gTraceOah->fTracePartGroups) {
        fLogOutputStream <<
          "Inserting part group descr " <<
          currentPartGroupDescr->
            partGroupDescrAsString () <<
          "' in pending list before" <<
          partGroupDescr->
            partGroupDescrAsString () <<
          ", line " << inputLineNumber <<
          endl;
      }
#endif

      startingPartGroupDescrsList.insert (
        i, partGroupDescr);
      break;
    }

    i++;
  } // while
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::insertPartGroupDescInStoppingList (
  int                  inputLineNumber,
  S_mxmlPartGroupDescr partGroupDescr,
  list<S_mxmlPartGroupDescr>&
                       stoppingPartGroupDescrsList)
{
  // this list is created in start position decreasing order,
  // so as to have the inner-most part group descr appearing first
  list<S_mxmlPartGroupDescr>::iterator
    iBegin = stoppingPartGroupDescrsList.begin (),
    iEnd   = stoppingPartGroupDescrsList.end (),
    i      = iBegin;

  while (true) {
    if (i == iEnd) {
      // append partGroupDescr to the list
#ifdef TRACE_OAH
      if (gTraceOah->fTracePartGroups) {
        fLogOutputStream <<
          "Appending part group descr " <<
          partGroupDescr->
            partGroupDescrAsString () <<
          "' at the end of the list of part group descrs stopping at position " <<
          fCurrentPartsPosition <<
          ", line " << inputLineNumber <<
          endl;
      }
#endif

      stoppingPartGroupDescrsList.push_back (
        partGroupDescr);
      /*
      stringstream s;
      s <<
        "part group descr " <<
        partGroupDescr->partGroupDescrAsString () <<
        " could not be inserted in part groups to be stopped list";

      msrInternalError (
        gOahOah->fInputSourceName,
        inputLineNumber,
        __FILE__, __LINE__,
        s.str ());
        */
      break;
    }

    // fetch current element in the list
    S_mxmlPartGroupDescr
      currentPartGroupDescr = (*i);

    if (
      partGroupDescr->getStartPosition ()
        >
      currentPartGroupDescr->getStartPosition ()
    ) {
      // insert partGroupDescr before currentPartGroupDescr
#ifdef TRACE_OAH
      if (gTraceOah->fTracePartGroups) {
        fLogOutputStream <<
          "Inserting part group descr " <<
          currentPartGroupDescr->
            partGroupDescrAsString () <<
          "' in pending list before" <<
          partGroupDescr->
            partGroupDescrAsString () <<
          ", line " << inputLineNumber <<
          endl;
      }
#endif

      stoppingPartGroupDescrsList.insert (
        i, partGroupDescr);
      break;
    }

    i++;
  } // while
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::registerPartGroupDescrAsStoppingAtCurrentPosition (
  int                  inputLineNumber,
  S_mxmlPartGroupDescr partGroupDescr)
{
  // fetch the list of part group descrs stopping at
  // the same position as partGroupDescr
  list<S_mxmlPartGroupDescr>&
    stoppingPartGroupDescrsList =
      fPositionStoppingPartGroupDescrsVector [
        fCurrentPartsPosition];

  if (stoppingPartGroupDescrsList.size () == 0) {
    // first element, insert it directly
    stoppingPartGroupDescrsList.push_back (
      partGroupDescr);
  }

  else {
    // insert the element at the right place in the stopping list
    insertPartGroupDescInStoppingList (
      inputLineNumber,
      partGroupDescr,
      stoppingPartGroupDescrsList);
  }

  // fetch the list of part group descrs starting at
  // the same position as partGroupDescr
  list<S_mxmlPartGroupDescr>&
    startingPartGroupDescrsList =
      fPositionStartingPartGroupDescrsVector [
        partGroupDescr->
          getStartPosition ()];

  if (startingPartGroupDescrsList.size () == 0) {
    // first element, insert it directly
    startingPartGroupDescrsList.push_back (
      partGroupDescr);
  }

  else {
    // insert partGroupDescr at the right place in the starting list
    insertPartGroupDescInStartingList (
      inputLineNumber,
      partGroupDescr,
      startingPartGroupDescrsList);
  }
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::registerPartGroupDescrAsStopped (
  int                  inputLineNumber,
  S_mxmlPartGroupDescr partGroupDescr)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Registering part group descr '" <<
      partGroupDescr->getPartGroupCombinedName () <<
      " as stopped" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // register part group descr as stopping at current position
  registerPartGroupDescrAsStoppingAtCurrentPosition (
    inputLineNumber,
    partGroupDescr);

  // forget it in fStartedPartGroupDescrsMap,
  // it remains in fAllPartGroupDescrsMap
  fStartedPartGroupDescrsMap.erase (
    partGroupDescr->
      getPartGroup ()->
        getPartGroupNumber ());

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "AFTER forgetting part group descr " +
        partGroupDescr->
          partGroupDescrAsString () +
        " in the data");
  }
#endif
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::handlePartGroupStart (
  int inputLineNumber)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Creating part group with number '" <<
      fCurrentPartGroupNumber <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fPartGroupsCounter++;

  // create the part group,
  // with the current part group as part group upLink
  S_msrPartGroup
    partGroupToBeStarted =
      msrPartGroup::create (
        inputLineNumber,
        fCurrentPartGroupNumber,
        fPartGroupsCounter,
        fCurrentPartGroupName,
        fCurrentPartGroupNameDisplayText,
        fCurrentPartGroupAccidentalText,
        fCurrentPartGroupAbbreviation,
        fCurrentPartGroupSymbolKind,
        fCurrentPartGroupSymbolDefaultX,
        fCurrentPartGroupBarlineKind,
        0, // partGroupPartGroupUpLink will be set upon 'stop'
        fMsrScore);

  // partGroupToBeStarted will be appended to the MSR score
  // upon 'stop', once it is complete

  // create the part group descr
  S_mxmlPartGroupDescr
    partGroupDescr =
      mxmlPartGroupDescr::create (
        inputLineNumber,
        fCurrentPartGroupNumber,
        partGroupToBeStarted,
        fCurrentPartsPosition) ;

  // register it in the part groups data
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Adding part group descr for '" << fCurrentPartGroupNumber <<
      "' to visitor's part groups data" <<
      ", fCurrentPartsPosition = " << fCurrentPartsPosition <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  registerPartGroupDescrAsStarted (
    inputLineNumber,
    partGroupDescr);

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "AFTER handlePartGroupStart()");
  }
#endif
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::handlePartGroupStop (
  int inputLineNumber)
{
  // is the part group to be stopped started?
  S_mxmlPartGroupDescr
    partGroupDescrToBeStopped =
      fetchStartedPartGroupDescr (
        fCurrentPartGroupNumber);

  if (! partGroupDescrToBeStopped) {
    // no, but we should have found it
    stringstream s;

    s <<
      "no part group '" << fCurrentPartGroupNumber <<
      "' has not been started, it cannot be stopped";

 // JMI   msrMusicXMLError (
    msrMusicXMLWarning (
      gOahOah->fInputSourceName,
      inputLineNumber,
  //    __FILE__, __LINE__,
      s.str ());
  }

  else {
    // set the stopping position
    partGroupDescrToBeStopped->
      setStopPosition (
        inputLineNumber,
        fCurrentPartsPosition);

    // register partGroupDescrToBeStopped as stopped
    registerPartGroupDescrAsStopped (
      inputLineNumber,
      partGroupDescrToBeStopped);
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "AFTER handlePartGroupStop()");
  }
#endif
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::doNestPartGroupDescrInItsContainer (
  int                  inputLineNumber,
  S_mxmlPartGroupDescr partGroupDescrToBeStopped,
  S_mxmlPartGroupDescr containingPartGroupDescr)
{
  // get part groups from descrs
  S_msrPartGroup
    partGroupToBeStopped =
      partGroupDescrToBeStopped->
        getPartGroup (),
    containingPartGroup =
      containingPartGroupDescr->
        getPartGroup ();

  // set currentPartGroup's upLink to containingPartGroupDescr
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Setting the upLink of part group " <<
      partGroupToBeStopped->
        getPartGroupCombinedName () <<
      "' to " <<
      containingPartGroup->
        getPartGroupCombinedName () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  partGroupToBeStopped->
    setPartGroupPartGroupUpLink (
      containingPartGroup);

  // appending currentPartGroup to containingPartGroupDescr
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Appending sub part group " <<
      partGroupToBeStopped->
        getPartGroupCombinedName () <<
      "' to " <<
      containingPartGroup->
        getPartGroupCombinedName () <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  containingPartGroup->
    appendSubPartGroupToPartGroup (
      partGroupToBeStopped);
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::createImplicitPartGroup ()
{
  // an implicit part group has to be created to contain everything,
  // since there can be parts out of any part group

  // sanity check
  msrAssert (
    fImplicitPartGroup == 0,
    "fImplicitPartGroup already exists");

  int inputLineNumber = 0;
    // this occurs independantly from the MusicXML data

  // create an implicit part group
  fCurrentPartGroupNumber = 0;

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Creating an implicit part group with number '" <<
      fCurrentPartGroupNumber <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fPartGroupsCounter++;

  fImplicitPartGroup =
    msrPartGroup::createImplicitPartGroup (
      fCurrentPartGroupNumber,
      fPartGroupsCounter,
      "Implicit", // partGroupName
      "",         // PartGroupNameDisplayText
      "",         // partGroupAccidentalText
      "Impl.",    // partGroupAbbreviation
      msrPartGroup::kPartGroupBarlineYes,
      fMsrScore);

  // append it to the MSR score
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Appending implicit part group '" <<
      fImplicitPartGroup->getPartGroupNumber () <<
      "' to MSR score" <<
      ", fCurrentPartsPosition = " << fCurrentPartsPosition <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  fMsrScore->
    addPartGroupToScore (
      fImplicitPartGroup);

  // create the part group descr for the implicit part group
  fImplicitPartGroupDescr =
    mxmlPartGroupDescr::create (
      inputLineNumber,
      fCurrentPartGroupNumber,
      fImplicitPartGroup,
      fCurrentPartsPosition);

  // register it in the part groups data
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Adding implicit part group descr for '" <<
      fCurrentPartGroupNumber <<
      "' to the part groups data" <<
      ", fCurrentPartsPosition = " << fCurrentPartsPosition <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  registerPartGroupDescrAsStarted (
    inputLineNumber,
    fImplicitPartGroupDescr);

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "AFTER creating fImplicitPartGroup");
  }
#endif
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::doPartGroupsNestingAndPartsAllocation (
  int inputLineNumber)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "doPartGroupsNestingAndPartsAllocation:" <<
      endl;
  }
#endif

  // this is actually a partial subpass of pass_2a,
  // since we run through the contents of <part-list />,
  // stored in the data we've built, a second time

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "BEFORE doPartGroupsNestingAndPartsAllocation");
  }
#endif

  // handle each position in turn
  for (int k = 0; k <= fCurrentPartsPosition; k++) {

    if (k > 0) {
      // parts actual positions start at 1
      // append part to current part group, i.e. to the top of the stack
      S_msrPart
        part =
          fPartsVector [k - 1];

      // fetch the part group descr stack top
      S_mxmlPartGroupDescr
        partGroupsDescrStackTop =
          fetchPartGroupDescrStackTop ();

      if (! partGroupsDescrStackTop) {
        stringstream s;

        s <<
          "Cannot append part " <<
          part->getPartCombinedName () <<
          " to any part group " <<
          " at position " << k <<
          ", since the stack is empty";

        msrMusicXMLError (
          gOahOah->fInputSourceName,
          inputLineNumber,
          __FILE__, __LINE__,
          s.str ());
      }

      // append part to current part group
      S_msrPartGroup
        currentPartGroup =
          partGroupsDescrStackTop->
            getPartGroup ();

      currentPartGroup->
        appendPartToPartGroup (
          part);
    }

    // handle the part groups descrs stopping at position k
    if (fPositionStoppingPartGroupDescrsVector.size ()) {
      list<S_mxmlPartGroupDescr>&
        stoppingPartGroupDescrsList =
          fPositionStoppingPartGroupDescrsVector [k];

      if (stoppingPartGroupDescrsList.size ()) {
        gIndenter++;

        list<S_mxmlPartGroupDescr>::const_iterator
          iBegin = stoppingPartGroupDescrsList.begin (),
          iEnd   = stoppingPartGroupDescrsList.end (),
          i      = iBegin;

        for ( ; ; ) {
          S_mxmlPartGroupDescr
            partGroupDescr = (*i);

          int stopInputLineNumber =
            partGroupDescr->getStopInputLineNumber ();

          S_msrPartGroup
            partGroup =
              partGroupDescr->getPartGroup ();

          // fetch the part group descr stack top
          S_mxmlPartGroupDescr
            partGroupsDescrStackTop =
              fetchPartGroupDescrStackTop ();

          if (! partGroupsDescrStackTop) {
            stringstream s;

            s <<
              "Cannot 'stop' part group descr " <<
              partGroupDescr->getPartGroupCombinedName () <<
              " at position " << k <<
              ", since the stack is empty";

            msrMusicXMLError (
              gOahOah->fInputSourceName,
              stopInputLineNumber,
              __FILE__, __LINE__,
              s.str ());
          }

          if (partGroupsDescrStackTop == partGroupDescr) {
            // pop partGroupDescrToBeStopped from the stack
            S_mxmlPartGroupDescr
              partGroupsDescrStackTop =
                fPartGroupsDescrStack.front ();

#ifdef TRACE_OAH
            if (gTraceOah->fTracePartGroups) {
              fLogOutputStream <<
                "Popping part group " <<
                partGroupDescr->
                  getPartGroupCombinedName () <<
                "' from the stack" <<
                ", line " << stopInputLineNumber <<
                endl;
            }
#endif

            fPartGroupsDescrStack.pop_front ();

            // the implicit part group isn't contained in any other
            if (partGroupDescr != fImplicitPartGroupDescr) {
              // fetch new current part group
              S_mxmlPartGroupDescr
                newPartGroupDescrStackTop =
                  fetchPartGroupDescrStackTop ();

              if (! newPartGroupDescrStackTop) {
                stringstream s;

                s <<
                  "there is no part group in the stack to nest part group descr " <<
                  partGroupDescr->partGroupDescrAsString () <<
                  fCurrentPartID << "\"" <<
                  " into";

                msrInternalError (
                  gOahOah->fInputSourceName,
                  stopInputLineNumber,
                  __FILE__, __LINE__,
                  s.str ());
              }

              // partGroupDescr is nested in newPartGroupDescrStackTop,
              // do the nesting
              doNestPartGroupDescrInItsContainer (
                stopInputLineNumber,
                partGroupDescr,
                newPartGroupDescrStackTop);
            }
          }

          else {
#ifdef TRACE_OAH
            if (gTraceOah->fTracePartGroups) {
              showPartGroupsData (
                stopInputLineNumber,
                "UPON overlapping part groups");
            }
#endif

            stringstream s;

            s <<
              endl <<
              "There are overlapping part groups, namely: " <<
              endl <<
              gTab << partGroupDescr->partGroupDescrAsString () <<
              endl <<
              "and" <<
              endl <<
              gTab << partGroupsDescrStackTop->partGroupDescrAsString () <<
              endl;

/* JMI
            // fetch the positions in the intersection
            int
              startOne =
                partGroupDescr->getStartPosition (),
              startTwo =
                partGroupsDescrStackTop->getStartPosition (),
              stopOne =
                partGroupDescr->getStopPosition (),
              stopTwo =
                partGroupsDescrStackTop->getStopPosition ();

            int firstCommonPosision = startOne;
            if (startTwo > startOne) {
              firstCommonPosision = startTwo;
            }

            int lastCommonPosision = stopOne;
            if (stopTwo < stopOne) {
              lastCommonPosision = stopTwo;
            }

            s <<
              endl <<
              "The parts they share are:" <<
              endl;

            s <<
              "(positions range is " <<
              firstCommonPosision << ".." << lastCommonPosision <<
              ")" <<
              endl;

            for (int m = firstCommonPosision; m <= lastCommonPosision; m++) {
              S_msrPart
                part =
                  fPartsVector [m];

              s <<
                gTab <<
                part->getPartCombinedName () <<
                ", line " << part->getInputLineNumber () <<
                endl;
            } // for
*/

            s <<
              endl <<
              replaceSubstringInString (
R"(Please contact the maintainers of libmusicxml2 (see option '-c, -contact'):
  either you found a bug in the EXECUTABLE translator,
  or this MusicXML data is the first-ever real-world case
  of a score exhibiting overlapping part groups.)",
              "EXECUTABLE",
              gOahOah->fHandlerExecutableName);

            msrMusicXMLError (
              gOahOah->fInputSourceName,
              stopInputLineNumber,
              __FILE__, __LINE__,
              s.str ());
          }

          if (++i == iEnd) break;
          // no endl here
        } // for

        gIndenter--;
      }
    }

    // handle the part groups descrs starting at position k
    if (fPositionStartingPartGroupDescrsVector.size ()) {
      list<S_mxmlPartGroupDescr>&
        startingPartGroupDescrsList =
          fPositionStartingPartGroupDescrsVector [k];

      if (startingPartGroupDescrsList.size ()) {
        gIndenter++;

        list<S_mxmlPartGroupDescr>::const_iterator
          iBegin = startingPartGroupDescrsList.begin (),
          iEnd   = startingPartGroupDescrsList.end (),
          i      = iBegin;

        for ( ; ; ) {
          S_mxmlPartGroupDescr
            partGroupDescr = (*i);

          S_msrPartGroup
            partGroup =
              partGroupDescr->getPartGroup ();

          // make it the new current part group
#ifdef TRACE_OAH
          if (gTraceOah->fTracePartGroups) {
            fLogOutputStream <<
              "Pushing part group descr '" <<
              partGroupDescr->partGroupDescrAsString () <<
              "' onto the part group stack" <<
              ", line " << inputLineNumber <<
              endl;
          }
#endif

          fPartGroupsDescrStack.push_front (
            partGroupDescr);

          if (++i == iEnd) break;
          // no endl here
        } // for

        gIndenter--;
      }
    }

#ifdef TRACE_OAH
    if (gTraceOah->fTracePartGroupsDetails) {
      stringstream s;

      s <<
        "AT position " << k;

      showPartGroupsData (
        inputLineNumber,
        s.str ());
    }
#endif

  } // for

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "AFTER doPartGroupsNestingAndPartsAllocation");
  }
#endif
}

//______________________________________________________________________________
S_msrStaff mxmlTree2MsrSkeletonBuilder::createStaffInCurrentPartIfNotYetDone (
  int inputLineNumber,
  int staffNumber)
{
  // is staffNumber already present in part?
  S_msrStaff
    staff =
      fCurrentPart->
        fetchStaffFromPart (staffNumber);

  if (! staff) {
    // no, add it to fCurrentPart
    staff =
      fCurrentPart->
        addStaffToPartByItsNumber (
          inputLineNumber,
          msrStaff::kStaffRegular,
          staffNumber);
  }

  return staff;
}

//______________________________________________________________________________
S_msrVoice mxmlTree2MsrSkeletonBuilder::createRegularVoiceInStaffIfNotYetDone (
  int inputLineNumber,
  int staffNumber,
  int voiceNumber)
{
  // the voice number is relative to a part,

  // create the staff if not yet done
  S_msrStaff
    staff =
      createStaffInCurrentPartIfNotYetDone (
        inputLineNumber,
        staffNumber);

  // is voice already present in staff?
  S_msrVoice
    voice =
      staff->
        fetchVoiceFromStaffByItsNumber (
          inputLineNumber,
          voiceNumber);

  if (! voice) {
    // create the voice and append it to the staff
    voice =
      staff->
        createVoiceInStaffByItsNumber (
          inputLineNumber,
          msrVoice::kVoiceRegular,
          voiceNumber,
          fCurrentMeasureNumber);
  }

  return voice;
}

//______________________________________________________________________________
S_msrVoice mxmlTree2MsrSkeletonBuilder::fetchFirstRegularVoiceFromStaff (
  int inputLineNumber,
  int staffNumber)
{
  // the voice number is relative to a part,

  // create the staff if not yet done
  S_msrStaff
    staff =
      createStaffInCurrentPartIfNotYetDone (
        inputLineNumber,
        staffNumber);

  // fetch the first regular
  S_msrVoice
    voice =
      staff->
        fetchFirstRegularVoiceFromStaff (
          inputLineNumber);

  return voice;
}

//______________________________________________________________________________
S_msrVoice mxmlTree2MsrSkeletonBuilder::createRegularVoiceHarmonyVoiceIfNotYetDone (
  int        inputLineNumber,
  S_msrVoice voice)
{
  // is the harmony voice already present in voice?
  S_msrVoice
    harmonyVoice =
      voice->
        getRegularVoiceHarmonyVoiceForwardLink ();

  if (! harmonyVoice) {
    // create the voice and append it to the staff
    harmonyVoice =
      voice->
        createRegularVoiceHarmonyVoice (
          inputLineNumber,
          fCurrentMeasureNumber);
  }

  return harmonyVoice;
}

S_msrVoice mxmlTree2MsrSkeletonBuilder::createPartHarmonyVoiceIfNotYetDone (
  int        inputLineNumber,
  S_msrPart  part)
{
  // is the harmony voice already present in voice?
  S_msrVoice
    partHarmonyVoice =
      part->
        getPartHarmoniesVoice ();

  if (! partHarmonyVoice) {
    // create the voice and append it to the staff
    partHarmonyVoice =
      part->
        createPartHarmonyVoice (
          inputLineNumber,
          fCurrentMeasureNumber);
  }

  return partHarmonyVoice;
}

//______________________________________________________________________________
S_msrVoice mxmlTree2MsrSkeletonBuilder::createRegularVoiceFiguredBassVoiceIfNotYetDone (
  int        inputLineNumber,
  S_msrVoice voice)
{
  // is the figured bass voice already present in voice?
  S_msrVoice
    figuredBassVoice =
      voice->
        getRegularVoiceFiguredBassVoiceForwardLink ();

  if (! figuredBassVoice) {
    // create the voice and append it to the staff
    figuredBassVoice =
      voice->
        createRegularVoiceFiguredBassVoice (
          inputLineNumber,
          fCurrentMeasureNumber);
  }

  return figuredBassVoice;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_score_partwise& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_score_partwise" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      "Analysing the score partwise" <<
      endl;
  }
#endif

  fScoreNumberOfMeasures = 0;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_score_partwise& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_score_partwise" <<
      ", line " << inputLineNumber <<
      endl;
  }

  // register the number of measures
  fMsrScore->
    setScoreNumberOfMeasures (
      fScoreNumberOfMeasures);

  // fetch the identification from the credits if any
  fMsrScore ->
    fetchIdentificationFromCreditsIfAny (
      inputLineNumber);
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_work_number& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_work_number" <<
      ", line " << inputLineNumber <<
      endl;
  }

  fMsrScore->getIdentification () ->
    setWorkNumber (
      inputLineNumber,
      elt->getValue ());
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_work_title& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_work_title" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string workTitle = elt->getValue ();

  fMsrScore->getIdentification () ->
    setWorkTitle (
      inputLineNumber,
      workTitle);
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_movement_number& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_movement_number" <<
      ", line " << inputLineNumber <<
      endl;
  }

  fMsrScore->getIdentification () ->
    setMovementNumber (
      inputLineNumber,
      elt->getValue ());
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_movement_title& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_movement_title" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string movementTitle = elt->getValue ();

  // remove HTML entities if any // JMI option for that?
/*
  convertHTMLEntitiesToPlainCharacters (
    movementTitle);
    */

  fMsrScore->getIdentification () ->
    setMovementTitle (
      inputLineNumber,
      movementTitle);
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_identification& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_identification" <<
      ", line " << inputLineNumber <<
      endl;
  }
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_identification& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_identification" <<
      ", line " << inputLineNumber <<
      endl;
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_creator& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_creator" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string creatorType = elt->getAttributeValue ("type");
  string creatorValue = elt->getValue ();

  if      (creatorType == "composer") {
    fMsrScore->getIdentification () ->
      addComposer (
        inputLineNumber,
        creatorValue);
  }

  else if (creatorType == "arranger") {
    fMsrScore->getIdentification () ->
      addArranger (
        inputLineNumber,
        creatorValue);
  }

  else if (creatorType == "lyricist") {
    fMsrScore->getIdentification () ->
      addLyricist (
        inputLineNumber,
        creatorValue);

    // should we use lyricists as poets?
    if (gMxmlTreeOah->fUseLyricistsAsPoets) {
      fMsrScore->getIdentification () ->
        addPoet (
          inputLineNumber,
          elt->getValue ());
    }
  }

  else if (creatorType == "poet") {
    fMsrScore->getIdentification () ->
      addPoet (
        inputLineNumber,
        elt->getValue ());
  }

  else if (creatorType == "translator") {
    fMsrScore->getIdentification () ->
      addTranslator (
        inputLineNumber,
        creatorValue);
  }

  else if (creatorType == "artist") {
    fMsrScore->getIdentification () ->
      addArtist (
        inputLineNumber,
        creatorValue);
  }

  else {
    stringstream s;

    s <<
      "creator type \"" << creatorType <<
      "\" is unknown";

// JMI    msrMusicXMLError (
    msrMusicXMLWarning (
      gOahOah->fInputSourceName,
      inputLineNumber,
//      __FILE__, __LINE__,
      s.str ());
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_rights& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_rights" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string rightsValue = elt->getValue ();

  convertHTMLEntitiesToPlainCharacters (rightsValue); // JMI &#x00a9;

  fMsrScore->getIdentification () ->
    addRights (
      inputLineNumber,
      rightsValue);
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_source& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_source" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  string rightsValue = elt->getValue ();

  convertHTMLEntitiesToPlainCharacters (rightsValue); // JMI &#x00a9;

  fMsrScore->getIdentification () ->
    addRights (
      inputLineNumber,
      rightsValue);
      */
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_relation& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_relation" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  string rightsValue = elt->getValue ();

  convertHTMLEntitiesToPlainCharacters (rightsValue); // JMI &#x00a9;

  fMsrScore->getIdentification () ->
    addRights (
      inputLineNumber,
      rightsValue);
      */
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_encoding& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_encoding" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      "*** Analysing S_encoding ***" <<
      ", " << elt->getValue () <<
      endl;
  }
#endif

  fScoreNumberOfMeasures = 0;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_encoding& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_encoding" <<
      ", line " << inputLineNumber <<
      endl;
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_encoding_date& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_encoding_date" <<
      ", line " << inputLineNumber <<
      endl;
  }

  fMsrScore->getIdentification () ->
    setEncodingDate (
      inputLineNumber,
      elt->getValue ());
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_encoder& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_encoder" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  fMsrScore->getIdentification () ->
    setEncodingDate (
      inputLineNumber,
      elt->getValue ());
      */
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_software& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_software" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string softwareValue = elt->getValue ();

  // convert clef to upper case for analysis
  string softwareValueToLower = softwareValue;

  transform (
    softwareValueToLower.begin (),
    softwareValueToLower.end (),
    softwareValueToLower.begin (),
    ::tolower);

  if (softwareValueToLower.find ("cubase") != string::npos) {
    msrMusicXMLWarning (
      gOahOah->fInputSourceName,
      inputLineNumber,
      "<software /> contains 'Cubase'");

    // is the '-cubase' option set???
    if (gMxmlTreeOah->fCubase) {
      S_oahElement
        cubaseOption =
          gMxmlTreeOah->
            getHandlerUpLink ()->
              fetchElementFromMap ("cubase");

      if (
        // combined booleans atom?
        S_oahCombinedBooleansAtom
          combinedBooleanAtom =
            dynamic_cast<oahCombinedBooleansAtom*>(&(*cubaseOption))
      ) {
        // handle it at once
#ifdef TRACE_OAH
        if (gTraceOah->fTraceOah) {
          fLogOutputStream <<
            "Setting '-cubase' option" <<
            endl;
        }
#endif

        combinedBooleanAtom->
          setCombinedBooleanVariables (true);
      }

      gMxmlTreeOah->fCubase = true;
    }
  }

  fMsrScore->getIdentification () ->
    addSoftware (
      inputLineNumber,
      softwareValue);
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_encoding_description& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_encoding_description" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  fMsrScore->getIdentification () ->
    setEncodingDate (
      inputLineNumber,
      elt->getValue ());
      */
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_supports& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_supports" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  fMsrScore->getIdentification () ->
    setEncodingDate (
      inputLineNumber,
      elt->getValue ());
      */
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_miscellaneous& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_miscellaneous" <<
      ", line " << inputLineNumber <<
      endl;
  }

/* JMI
  string miscellaneousFielValue = elt->getValue ();

  convertHTMLEntitiesToPlainCharacters (
    miscellaneousFielValue);

  fMsrScore->getIdentification () ->
    setMiscellaneousField (
      inputLineNumber,
      miscellaneousFielValue);
      */
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_miscellaneous& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_miscellaneous" <<
      ", line " << inputLineNumber <<
      endl;
  }
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_miscellaneous_field& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_miscellaneous_field" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string miscellaneousFielValue = elt->getValue ();

  convertHTMLEntitiesToPlainCharacters (
    miscellaneousFielValue);

  fMsrScore->getIdentification () ->
    setMiscellaneousField (
      inputLineNumber,
      miscellaneousFielValue);
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_millimeters& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_millimeters" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentMillimeters = (float)(*elt);

  fMsrScore->
    getScaling ()->
      setMillimeters (fCurrentMillimeters);
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_tenths& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_tenths" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentTenths = (int)(*elt);

  fMsrScore->
    getScaling ()->
      setTenths (fCurrentTenths);
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_scaling& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_scaling" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTraceGeometry) {
    fLogOutputStream <<
      "There are " << fCurrentTenths <<
      " tenths for " <<  fCurrentMillimeters <<
      " millimeters, hence the global staff size is " <<
      fMsrScore->getScaling ()->fetchGlobalStaffSize () <<
      endl;
  }
#endif
}

//______________________________________________________________________________
/* JMI
        <system-layout>
          <system-margins>
            <left-margin>21.00</left-margin>
            <right-margin>0.00</right-margin>
            </system-margins>
          <top-system-distance>170.00</top-system-distance>
        </system-layout>
*/

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_system_layout& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_system_layout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingSystemLayout = true;
}
void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_system_layout& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_system_layout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingSystemLayout = false;
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_system_margins& elt )
{
/*
        <system-layout>
          <system-margins>
            <left-margin>0.00</left-margin>
            <right-margin>-0.00</right-margin>
            </system-margins>
          <top-system-distance>170.00</top-system-distance>
          </system-layout>
*/

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_system_margins" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (! fOnGoingSystemLayout) {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<system-margins /> is out of context");
  }

  fOnGoingSystemMargins = true;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_system_margins& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_system_margins" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (! fOnGoingSystemLayout) {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<system-margins /> is out of context");
  }

  fOnGoingSystemMargins = false;
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_system_distance& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_system_distance" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingSystemLayout) {
/*
    float systemDistanceTenths = (float)(*elt);

    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setBetweenSystemSpace (
            msrLength::create (
              kMillimeterUnit,
              systemDistanceTenths * fCurrentMillimeters / fCurrentTenths));
              */
    stringstream s;

    s <<
      "<system-distance /> is not supported yet by " <<
      gOahOah->fHandlerExecutableName;

/* JMI
    msrMusicXMLWarning (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
     s.str ());
     */
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<system-distance /> is out of context");
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_top_system_distance& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_top_system_distance" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingSystemLayout) {
/*
    float topSystemDistanceTenths = (float)(*elt);

    fMsrScore->
      getScaling ()->
        setPageTopSpace (
          msrLength::create (
            kMillimeterUnit,
            topSystemDistanceTenths * fCurrentMillimeters / fCurrentTenths));
          */
    stringstream s;

    s <<
      "<top-system-distance /> is not supported yet by " <<
      gOahOah->fHandlerExecutableName;

/* JMI
    msrMusicXMLWarning (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
     s.str ());
     */
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<top-system-distance /> is out of context");
  }
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_system_dividers& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_system_dividers" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

    stringstream s;

    s <<
      "<system-dividers /> is not supported yet by " <<
      gOahOah->fHandlerExecutableName;

/* JMI
    msrMusicXMLWarning (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
     s.str ());
     */
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_left_divider& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_left_divider" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_right_divider& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_right_divider" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }
}

//______________________________________________________________________________
/* JMI
    <page-layout>
      <page-height>1683.36</page-height>
      <page-width>1190.88</page-width>
      <page-margins type="even">
        <left-margin>56.6929</left-margin>
        <right-margin>56.6929</right-margin>
        <top-margin>56.6929</top-margin>
        <bottom-margin>113.386</bottom-margin>
        </page-margins>
      <page-margins type="odd">
        <left-margin>56.6929</left-margin>
        <right-margin>56.6929</right-margin>
        <top-margin>56.6929</top-margin>
        <bottom-margin>113.386</bottom-margin>
        </page-margins>
      </page-layout>
*/

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_page_layout& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_page_layout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPageLayout = true;
}
void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_page_layout& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_page_layout" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPageLayout = false;
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_page_height& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_page_height" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingPageLayout) {
    float pageHeight = (float)(*elt);

    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setPageHeight (
            msrLength::create (
              kMillimeterUnit,
              pageHeight * fCurrentMillimeters / fCurrentTenths));
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<page-height /> is out of context");
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_page_width& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_page_width" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingPageLayout) {
    float pageWidth = (float)(*elt);

    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setPageWidth (
            msrLength::create (
              kMillimeterUnit,
              pageWidth * fCurrentMillimeters / fCurrentTenths));
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<page-width /> is out of context");
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_page_margins& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_page_margins" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingPageLayout) {
    string pageMarginsType =
      elt->getAttributeValue ("type");

    fCurrentMarginTypeKind = kBothMargins; // default value

    if      (pageMarginsType == "odd")
      fCurrentMarginTypeKind = kOddMargin;
    else if (pageMarginsType == "even")
      fCurrentMarginTypeKind = kEvenMargin;
    else if (pageMarginsType == "both")
      fCurrentMarginTypeKind = kBothMargins;
    else if (pageMarginsType. size ()) {
      stringstream s;

      s <<
        "unknown page margins type \"" <<
        pageMarginsType <<
        "\"";

      msrMusicXMLError (
        gOahOah->fInputSourceName,
        elt->getInputLineNumber (),
        __FILE__, __LINE__,
        s.str ());
    }
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<page-margins /> is out of context");
  }

  fOnGoingPageMargins = true;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_page_margins& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_page_margins" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPageMargins = false;
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_left_margin& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_left_margin" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingPageMargins) {
    float leftMargin = (float)(*elt);

    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setLeftMargin (
            msrMargin::create (
              fCurrentMarginTypeKind,
              msrLength (
                kMillimeterUnit,
                leftMargin * fCurrentMillimeters / fCurrentTenths)));
  }
  else if (fOnGoingSystemMargins) {

/* JMI
    float leftMargin = (float)(*elt);
    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setLeftMargin (
            msrMargin::create (
              fCurrentMarginTypeKind,
              msrLength (
                kMillimeterUnit,
                leftMargin * fCurrentMillimeters / fCurrentTenths)));
                */
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<left-margin /> is out of context");
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_right_margin& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_right_margin" <<
       ", line " << elt->getInputLineNumber () <<
     endl;
  }

  if (fOnGoingPageMargins) {
    float rightMargin = (float)(*elt);

    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setRightMargin (
            msrMargin::create (
              fCurrentMarginTypeKind,
              msrLength (
                kMillimeterUnit,
                rightMargin * fCurrentMillimeters / fCurrentTenths)));
  }
  else if (fOnGoingSystemMargins) {

/* JMI
    float rightMargin = (float)(*elt);
    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setRightMargin (
            msrMargin::create (
              fCurrentMarginTypeKind,
              msrLength (
                kMillimeterUnit,
                rightMargin * fCurrentMillimeters / fCurrentTenths)));
                */
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<right-margin /> is out of context");
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_top_margin& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_top_margin" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingPageMargins) {
    float topMargin = (float)(*elt);

    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setTopMargin (
            msrMargin::create (
              fCurrentMarginTypeKind,
              msrLength (
                kMillimeterUnit,
                topMargin * fCurrentMillimeters / fCurrentTenths)));
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<top-margin /> is out of context");
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_bottom_margin& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_bottom_margin" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  if (fOnGoingPageMargins) {
    float bottomMargin = (float)(*elt);

    fMsrScore->
      getScaling ()->
        getPageLayout ()->
          setBottomMargin (
            msrMargin::create (
              fCurrentMarginTypeKind,
              msrLength (
                kMillimeterUnit,
                bottomMargin * fCurrentMillimeters / fCurrentTenths)));
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      elt->getInputLineNumber (),
      __FILE__, __LINE__,
      "<bottom-margin /> is out of context");
  }
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_credit& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_credit" <<
      ", line " << inputLineNumber <<
      endl;
  }

  int creditPageNumber =
    elt->getAttributeIntValue ("page", 0);

  fCurrentCredit =
    msrCredit::create (
      inputLineNumber,
      creditPageNumber);
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_credit_words& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_credit_words" <<
      ", line " << inputLineNumber <<
      endl;
  }

/*
  <credit page="1">
    <credit-words default-x="548" default-y="1382" font-family="FreeSerif" font-size="26" font-weight="bold" justify="center" valign="top" xml:space="preserve">"R E Q U I E M"    from    D E A T H N O T E</credit-words>
  </credit>
  <credit page="1">
    <credit-words default-x="548" default-y="1331" font-family="FreeSerif" font-size="16" font-style="italic" justify="center" valign="top">Theme from L's death. For SATB choir.</credit-words>
  </credit>
  <credit page="1">
    <credit-words default-x="1046" default-y="1253" font-family="FreeSerif" font-size="12" justify="right" valign="bottom">Yoshihisa Hirano &amp; Hideki Taniuchi</credit-words>
  </credit>
  <credit page="1">
    <credit-words default-x="51" default-y="1253" font-family="FreeSerif" font-size="12" valign="bottom" xml:lang="fr">arrangement and lyrics by Andrés Rojas</credit-words>
  </credit>
  */

  string creditWordsContents =
    elt->getValue ();

  // font family
  string creditWordsFontFamily =
    elt->getAttributeValue ("font-family");

  // font size
  float creditWordsFontSize =
    elt->getAttributeFloatValue ("font-size", 0.0);

  // font weight
  string creditWordsFontWeightString =
    elt->getAttributeValue ("font-weight"); // JMI etc

  msrFontWeightKind
    creditWordsFontWeightKind =
      msrFontWeightKindFromString (
        inputLineNumber,
        creditWordsFontWeightString);

  // font style
  string creditWordsFontStyleString =
    elt->getAttributeValue ("font-style");

  msrFontStyleKind
    creditWordsFontStyleKind =
      msrFontStyleKindFromString (
        inputLineNumber,
        creditWordsFontStyleString);

  // justify
  string creditWordsJustifyString =
    elt->getAttributeValue ("justify");

  msrJustifyKind
    creditWordsJustifyKind =
      msrJustifyKindFromString (
        inputLineNumber,
        creditWordsJustifyString);

  // halign
  string creditWordsHAlignString =
    elt->getAttributeValue ("halign");

  msrHorizontalAlignmentKind
    creditWordsHorizontalAlignment =
      msrHorizontalAlignmentKindFromString (
        inputLineNumber,
        creditWordsHAlignString);

  // valign
  string creditWordsVAlignString =
    elt->getAttributeValue ("valign");

  msrVerticalAlignmentKind
    creditWordsVerticalAlignment =
      msrVerticalAlignmentKindFromString (
        inputLineNumber,
        creditWordsVAlignString);

  // XMLLang
  string creditWordsXMLLangString =
    elt->getAttributeValue ("xml:lang");

  msrXMLLangKind
    creditWordsXMLLang =
      msrXMLLangKindFromString (
        inputLineNumber,
        creditWordsXMLLangString);

  // create the credit words
  S_msrCreditWords
    creditWords =
      msrCreditWords::create (
        inputLineNumber,
        creditWordsContents,
        creditWordsFontFamily,
        creditWordsFontSize,
        creditWordsFontWeightKind,
        creditWordsFontStyleKind,
        creditWordsJustifyKind,
        creditWordsHorizontalAlignment,
        creditWordsVerticalAlignment,
        creditWordsXMLLang);

  // append it to the current credit
  fCurrentCredit->
    appendCreditWordsToCredit (
      creditWords);
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_credit& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_credit" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fMsrScore->
    appendCreditToScore (fCurrentCredit);

  fCurrentCredit = nullptr;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_part_list& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_part_list" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      "Analysing part list" <<
      endl;
  }
#endif

  gIndenter++;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_part_list& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_part_list" <<
      ", line " << inputLineNumber <<
      endl;
  }

  gIndenter--;

  // set implicit part group descr end position
  fImplicitPartGroupDescr->
    setStopPosition (
      inputLineNumber, // JMI ???
      INT_MAX);

  // register implicit part group descr as stopped
  registerPartGroupDescrAsStopped (
    inputLineNumber,
    fImplicitPartGroupDescr);

  // do the job...
  doPartGroupsNestingAndPartsAllocation (
    inputLineNumber);

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    showPartGroupsData (
      inputLineNumber,
      "Part groups data gathered for score skeleton");
  }
#endif

  if (gMsrOah->fDisplayPartGroups) {
    fLogOutputStream <<
      endl <<
      "The implicit part group contains:" <<
      endl;

    gIndenter++;

    fImplicitPartGroup->
      printPartGroupParts (
        inputLineNumber,
        fLogOutputStream);

    gIndenter--;
  }
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_part_group& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_part_group" <<
      ", line " << inputLineNumber <<
      endl;
  }

  // part group number

  fCurrentPartGroupNumber =
    elt->getAttributeIntValue ("number", 0);

  // part group type

  string partGroupType = elt->getAttributeValue ("type");

  fCurrentPartGroupTypeKind = msrPartGroup::kPartGroupTypeNone;

  if      (partGroupType == "start")
    fCurrentPartGroupTypeKind = msrPartGroup::kPartGroupTypeStart;
  else if (partGroupType == "stop")
    fCurrentPartGroupTypeKind = msrPartGroup::kPartGroupTypeStop;
  else {
    stringstream s;

    s <<
      "unknown part group type \"" <<
      partGroupType <<
      "\"";

    msrMusicXMLError (
      gOahOah->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }

  // part group print style

  string printStyle = elt->getAttributeValue ("print-style"); // JMI

  // part group print object

  string printObject = elt->getAttributeValue ("print-object"); // JMI

  // handle part group type
  switch (fCurrentPartGroupTypeKind) {

    case msrPartGroup::kPartGroupTypeStart:
      fCurrentPartGroupName = "";
      fCurrentPartGroupNameDisplayText = "";
      fCurrentPartGroupAccidentalText = "";
      fCurrentPartGroupAbbreviation = "";
      fCurrentPartGroupSymbolKind = msrPartGroup::kPartGroupSymbolNone;
      fCurrentPartGroupSymbolDefaultX = INT_MIN;
      fCurrentPartGroupBarlineKind = msrPartGroup::kPartGroupBarlineYes;
      break;

    case msrPartGroup::kPartGroupTypeStop:
      break;

    case msrPartGroup::kPartGroupTypeNone:
      // should not occur
      break;
  } // switch
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_group_name& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_group_name" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentPartGroupName = elt->getValue();
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_group_name_display& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_group_name_display" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPartGroupNameDisplay = true;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_group_name_display& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_group_name_display" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPartGroupNameDisplay = false;
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_display_text& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_display_text" <<
      ", line " << inputLineNumber <<
      endl;
  }

  if (fOnGoingPartGroupNameDisplay) {
    fCurrentPartGroupNameDisplayText =
      elt->getValue ();
  }
  else if (fOnGoingPartNameDisplay) {
    fCurrentPartNameDisplayText =
      elt->getValue ();
  }
  else if (fOnGoingPartAbbreviationDisplay) {
    fCurrentPartAbbreviationDisplayText =
      elt->getValue ();
  }
  else {
    msrMusicXMLError (
      gOahOah->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      "<display-text /> is out of context");
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_accidental_text& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_accidental_text" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

    fCurrentPartGroupAccidentalText = elt->getValue();
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_group_abbreviation& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_group_abbreviation" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentPartGroupAbbreviation = elt->getValue ();
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_group_symbol& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_group_symbol" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string groupSymbol = elt->getValue ();

  fCurrentPartGroupSymbolKind =
    msrPartGroup::kPartGroupSymbolNone; // default value

  if      (groupSymbol == "brace")
    fCurrentPartGroupSymbolKind = msrPartGroup::kPartGroupSymbolBrace;

  else if (groupSymbol == "bracket")
    fCurrentPartGroupSymbolKind = msrPartGroup::kPartGroupSymbolBracket;

  else if (groupSymbol == "line")
    fCurrentPartGroupSymbolKind = msrPartGroup::kPartGroupSymbolLine;

  else if (groupSymbol == "square")
    fCurrentPartGroupSymbolKind = msrPartGroup::kPartGroupSymbolSquare;

  else if (groupSymbol == "none")
    fCurrentPartGroupSymbolKind = msrPartGroup::kPartGroupSymbolNone;

  else {
    if (groupSymbol.size ()) {
      // part group type may be absent
      stringstream s;

      s <<
        "unknown part group symbol \"" + groupSymbol + "\"";

      msrMusicXMLError (
        gOahOah->fInputSourceName,
        inputLineNumber,
        __FILE__, __LINE__,
        s.str ());
    }
  }

  fCurrentPartGroupSymbolDefaultX =
    elt->getAttributeIntValue ("default-x", 0);
}

void mxmlTree2MsrSkeletonBuilder::visitStart ( S_group_barline& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_group_barline" <<
      ", line " << inputLineNumber <<
      endl;
  }

  string groupBarline = elt->getValue ();

  // check part group barline
  if      (groupBarline == "yes")
    fCurrentPartGroupBarlineKind = msrPartGroup::kPartGroupBarlineYes;
  else if (groupBarline == "no")
    fCurrentPartGroupBarlineKind = msrPartGroup::kPartGroupBarlineNo;
  else {
    stringstream s;

    s <<
      "unknown part group barline \"" + groupBarline + "\"";

    msrMusicXMLError (
      gOahOah->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitEnd (S_part_group& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_part_group" <<
      ", line " << inputLineNumber <<
      endl;
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Handling part group '" <<
      fCurrentPartGroupNumber <<
      "', type: \"" <<
      msrPartGroup::partGroupTypeKindAsString (
        fCurrentPartGroupTypeKind) << "\""  <<
      ", fCurrentPartsPosition = " << fCurrentPartsPosition <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  gIndenter++;

  // handle part group type
  switch (fCurrentPartGroupTypeKind) {

    case msrPartGroup::kPartGroupTypeStart:
      // handle the part group start
      handlePartGroupStart (
        inputLineNumber);
      break;

    case msrPartGroup::kPartGroupTypeStop:
      // handle the part group stop
      handlePartGroupStop (
        inputLineNumber);
      break;

    case msrPartGroup::kPartGroupTypeNone:
      // should not occur
      break;
  } // switch

  gIndenter--;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_score_part& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_score_part" <<
      ", line " << inputLineNumber <<
      endl;
  }

  fCurrentPartID = elt->getAttributeValue ("id");

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      "Found part name \"" << fCurrentPartID << "\"" <<
      " in part list" <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // is the part id a pure number?
  string regularExpression (
    "[[:space:]]*"
    "[[:digit:]]+"
    "[[:space:]]*"
    );

  regex  e (regularExpression);
  smatch sm;

  regex_match (fCurrentPartID, sm, e);

  if (sm.size () == 1) {
    stringstream s;

    s <<
      "Part name \"" << fCurrentPartID << "\"" <<
      " is a pure number" <<
      ", line " << inputLineNumber;

    msrMusicXMLWarning (
      gOahOah->fInputSourceName,
      inputLineNumber,
      s.str ());
  }

  // initializs fields
  fCurrentPartName = "";
  fCurrentPartNameDisplayText = "";

  fCurrentPartAbbreviation = "";
  fCurrentPartAbbreviationDisplayText = "";

  fCurrentPartInstrumentName = "";
  fCurrentPartInstrumentAbbreviation = "";
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_part_name& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_part_name" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentPartName = elt->getValue ();

  string printStyle = elt->getAttributeValue ("print-style"); // JMI
  if (printStyle == "JMI") {
  }

  string printObject = elt->getAttributeValue ("print-object"); // JMI
  if (printObject == "JMI") {
  }
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_part_name_display& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_part_name_display" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPartNameDisplay = true;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_part_name_display& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_part_name_display" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPartNameDisplay = false;
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_part_abbreviation& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_part_abbreviation" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentPartAbbreviation = elt->getValue ();
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_part_abbreviation_display& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_part_abbreviation_display" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPartAbbreviationDisplay = true;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_part_abbreviation_display& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_part_abbreviation_display" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fOnGoingPartAbbreviationDisplay = false;
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_instrument_name& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_instrument_name" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentPartInstrumentName = elt->getValue();
}

void mxmlTree2MsrSkeletonBuilder::visitStart (S_instrument_abbreviation& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_instrument_abbreviation" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentPartInstrumentAbbreviation = elt->getValue();
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_score_part& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_score_part" <<
      ", line " << inputLineNumber <<
      endl;
  }

  fCurrentPartsPosition++;

  string partID = elt->getAttributeValue ("id");

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
   fLogOutputStream <<
    "--------------------------------------------" <<
    endl <<
    "Handling score part name \"" << fCurrentPartID << "\"" <<
    ", fCurrentPartsPosition = " << fCurrentPartsPosition <<
    ", line " << inputLineNumber <<
    endl;
  }
#endif

  gIndenter++;

  // create the part
#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
   fLogOutputStream <<
    "--------------------------------------------" <<
    endl <<
    "Creating part \"" << fCurrentPartID << "\"" <<
    ", line " << inputLineNumber <<
    endl;
  }
#endif

  S_msrPart
    part =
      msrPart::create (
        inputLineNumber,
        fCurrentPartID,
        0); // partPartGroupUpLink will be set later

  // populate it
  part->
    setPartName (
      fCurrentPartName);
  part->
    setPartNameDisplayText (
      fCurrentPartNameDisplayText);

  part->
    setPartAbbreviation (
      fCurrentPartAbbreviation);
  part->
    setPartAbbreviationDisplayText (
      fCurrentPartAbbreviationDisplayText);

  part->
    setPartInstrumentName (
      fCurrentPartInstrumentName);
  part->
    setPartInstrumentAbbreviation (
      fCurrentPartInstrumentAbbreviation);

  // register part in the parts data
  registerPart (
    inputLineNumber,
    fCurrentPartsPosition,
    part);

#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroupsDetails) {
    showPartGroupsData (
      inputLineNumber,
      "AFTER handling score part \"" + partID + "\"");
  }
#endif

  gIndenter--;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::registerPart (
  int       inputLineNumber,
  int       partPosition,
  S_msrPart part)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTracePartGroups) {
    fLogOutputStream <<
      "Registering part " <<
      part->getPartCombinedName () <<
      " in the parts data" <<
      ", partPosition = " << partPosition <<
      ", line " << inputLineNumber <<
      endl;
  }
#endif

  // register part in the parts vector
  fPartsVector.push_back (part);

  // register it in the parts map
  fPartsMap [part->getPartID ()] = part;

  // create an empty list for part groups starting at partPosition
  fPositionStartingPartGroupDescrsVector.push_back (
    list<S_mxmlPartGroupDescr> ());

  // create an empty list for part groups stopping at partPosition
  fPositionStoppingPartGroupDescrsVector.push_back (
    list<S_mxmlPartGroupDescr> ());
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_part& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_part" <<
      ", line " << inputLineNumber <<
      endl;
  }

  // fCurrentPartID is used throughout
  fCurrentPartID = elt->getAttributeValue ("id");

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      endl <<
      "<!--=== part \"" << fCurrentPartID << "\"" <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }
#endif

  if (! fCurrentPartID.size ()) {
    if (fPartsMap.size () == 1) {
      // there's only one part in the part list,
      // assume this is the one
      fCurrentPartID =
        (*fPartsMap.begin ()).second->
          getPartID ();

      stringstream s;

      s <<
        "part id is empty, using '" <<
        fCurrentPartID <<
        "' since it is the only part in the <part-list />";

      msrMusicXMLWarning (
        gOahOah->fInputSourceName,
        inputLineNumber,
        s.str ());
    }

    else {
      fCurrentNoIDPartNumber++;

      stringstream s;

      s <<
        "NO_ID_PART_" << fCurrentNoIDPartNumber;

      s >> fCurrentPartID;

      msrMusicXMLWarning (
        gOahOah->fInputSourceName,
        inputLineNumber,
        "part 'id' is empty, creating one as '" + fCurrentPartID + "'");
    }
  }

  // is this part already known?
  if (fPartsMap.count (fCurrentPartID)) {
    fCurrentPart = // used thoughoutfRepeatHasBeenCreatedForCurrentPart
      fPartsMap [fCurrentPartID];
  }

  else {
    stringstream s;

    s <<
      "part \"" <<
      fCurrentPartID <<
      "\" is not known in the <part-list />";

    msrMusicXMLError (
      gOahOah->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }

  gIndenter++;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      endl <<
      "--------------------------------------------" <<
      endl <<
      "Analyzing part \"" << fCurrentPartID << "\" -- start" <<
      endl;
  }
#endif

  // measures
  fPartNumberOfMeasures = 0;

  // staves and voices
  fCurrentStaffMusicXMLNumber = 1; // default if there are no <staff> element
  fCurrentVoiceMusicXMLNumber = 1; // default if there are no <voice> element
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_part& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_part" <<
      ", line " << inputLineNumber <<
      endl;
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTraceParts) {
    fLogOutputStream <<
      "Analyzing part \"" << fCurrentPartID << "\" -- end" <<
      endl <<
      "--------------------------------------------" <<
      endl;
  }
#endif

  gIndenter--;

  // handle current part number of measures
  if (fScoreNumberOfMeasures == 0) {
    fScoreNumberOfMeasures =
      fPartNumberOfMeasures;
  }
  else {
    // is the part number of measures consistent?
    if (fPartNumberOfMeasures != fScoreNumberOfMeasures) {
      stringstream s;

      s <<
        "part " << fCurrentPart->getPartCombinedName () <<
        " has " << fPartNumberOfMeasures <<
        " measures while the other ones have " << fScoreNumberOfMeasures;

      msrMusicXMLError (
        gOahOah->fInputSourceName,
        inputLineNumber,
      __FILE__, __LINE__,
        s.str ());
    }
  }

  // set current part's number of measures
  fCurrentPart->
    setPartNumberOfMeasures (
      fPartNumberOfMeasures);

  // are there more staves in <staves/> that specified with <stave/>?
  fCurrentPart->
    addAVoiceToStavesThatHaveNone (
      inputLineNumber);

  // assign sequential numbers to the voices in the part
  fCurrentPart->
    assignSequentialNumbersToRegularVoicesInPart (
      inputLineNumber);
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_staves& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_direction" <<
      ", line " << inputLineNumber <<
      endl;
  }

  int stavesNumber = int(*elt);

#ifdef TRACE_OAH
  if (gTraceOah->fTraceStaves) {
    switch (stavesNumber) {
      case 0:
        fLogOutputStream <<
          "There isn't any explicit staff (hence 1 by default)"; // JMI
        break;

      case 1:
        fLogOutputStream <<
          "There is 1 staff";
        break;

      default:
        fLogOutputStream <<
          "There are " << stavesNumber << " staves";
    } // switch

    fLogOutputStream <<
      " in part " << fCurrentPart->getPartCombinedName() <<
      endl;
  }
#endif

  if (stavesNumber > 1) {
    // add stavesNumber staves to current part
    int n = 1;

    while (n <= stavesNumber) {
    /* JMI
      fCurrentPart->
        addStaffToPartByItsNumber (
          inputLineNumber,
          msrStaff::kStaffRegular,
          n);
          */
      S_msrStaff
        dummyStaff = // JMI
          createStaffInCurrentPartIfNotYetDone (
            inputLineNumber,
            n);

      n++;
    } // while
  }
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_staff& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_staff" <<
      ", line " << inputLineNumber <<
      endl;
  }

  fCurrentStaffMusicXMLNumber = int(*elt);

  // the staff number should be positive
  if (fCurrentStaffMusicXMLNumber <= 0) {
    stringstream s;

    s <<
      "staff number " << fCurrentStaffMusicXMLNumber <<
      " is not positive" <<
      ", line " << inputLineNumber;

    msrAssert (false, s.str ());
  }

  if (fOnGoingNote) { // JMI
    // regular staff indication in note/rest, fine
  }
  else {
    // JMI ???
  }
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_voice& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_staff_details" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  fCurrentVoiceMusicXMLNumber = int(*elt);

  if (fOnGoingNote) { // JMI
    // regular voice indication in note/rest, fine
  }
  else {
    // JMI ???
  }

  // don't attempt to create the voice now,
  // it's staff number if any comes later!
  // do it upton visitEnd ( S_note& )
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_measure& elt)
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_measure" <<
      ", line " << inputLineNumber <<
      endl;
  }

/*
  Measures have a required number attribute (going from partwise to timewise,
  measures are grouped via the number).
*/

  fCurrentMeasureNumber = // JMI local variable???
    elt->getAttributeValue ("number");

#ifdef TRACE_OAH
  if (gTraceOah->fTraceMeasures) {
    fLogOutputStream <<
      endl <<
      "<!--=== measure " << fCurrentMeasureNumber <<
      ", line " << inputLineNumber << " ===-->" <<
      endl;
  }
#endif

  // take this measure into account
  fPartNumberOfMeasures++;

  gIndenter++;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd (S_measure& elt)
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_measure" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  gIndenter--;
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_note& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_note" <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  // assuming staff number 1, unless S_staff states otherwise afterwards
  fCurrentStaffMusicXMLNumber = 1;

  // assuming voice number 1, unless S_voice states otherwise afterwards
  fCurrentVoiceMusicXMLNumber = 1;

  // lyrics
  fCurrentStanzaNumber = K_NO_STANZA_NUMBER;
  fCurrentStanzaName = K_NO_STANZA_NAME;

  fOnGoingNote = true;
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_note& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_note" <<
      ", line " << inputLineNumber <<
      endl;
  }

  // should the staff be created?
  S_msrStaff
    staff =
      createStaffInCurrentPartIfNotYetDone (
        inputLineNumber,
        fCurrentStaffMusicXMLNumber);

  // should the voice be created?
  S_msrVoice
    noteVoice =
      createRegularVoiceInStaffIfNotYetDone (
        inputLineNumber,
        fCurrentStaffMusicXMLNumber,
        fCurrentVoiceMusicXMLNumber);

#ifdef TRACE_OAH
  if (gTraceOah->fTraceNotes) {
    fLogOutputStream <<
      "--> S_note, fCurrentStaffMusicXMLNumber = " <<
      fCurrentStaffMusicXMLNumber <<
      endl <<
      "--> S_note, current staff name  = " <<
      staff->getStaffName() <<
      endl <<
      "--> S_note, fCurrentVoiceMusicXMLNumber        = " <<
      fCurrentVoiceMusicXMLNumber <<
      endl <<
      "--> S_note, current noteVoice name  = " <<
      noteVoice->getVoiceName() <<
      endl;
  }
#endif

  // are there harmonies attached to the current note?
  if (fThereAreHarmoniesToBeAttachedToCurrentNote) {
    if (gMsrOah->fOmitHarmonies) {
#ifdef TRACE_OAH
      if (gTraceOah->fTraceHarmonies) {
        fLogOutputStream <<
          "Omitting the harmonies" <<
          ", line " <<
          inputLineNumber <<
          endl;
      }
#endif
    }
    else {
      // create the harmony voice
      S_msrVoice
        harmonyVoice =
          createRegularVoiceHarmonyVoiceIfNotYetDone (
            inputLineNumber,
            noteVoice);

/* JMI
      // create the part harmony voice
      S_msrVoice
        partHarmonyVoice =
          createPartHarmonyVoiceIfNotYetDone (
            inputLineNumber,
            fCurrentPart);
            */
    }

    fThereAreHarmoniesToBeAttachedToCurrentNote = false;
  }

  // are there figured bass attached to the current note?
  if (fThereAreFiguredBassToBeAttachedToCurrentNote) {
    if (gMsrOah->fOmitFiguredBasses) {
#ifdef TRACE_OAH
      if (gTraceOah->fTraceFiguredBasses) {
        fLogOutputStream <<
          "Omitting the figured basses" <<
          ", line " <<
          inputLineNumber <<
          endl;
      }
#endif
    }
    else {
      // should the figured bass voice be created?
      S_msrVoice
        figuredBassVoice =
          createRegularVoiceFiguredBassVoiceIfNotYetDone (
            inputLineNumber,
            noteVoice);
    }

    fThereAreFiguredBassToBeAttachedToCurrentNote = false;
  }

  fOnGoingNote = false;
}

//________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart (S_lyric& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_lyric" <<
      ", line " << inputLineNumber <<
      endl;
  }

  // number

  {
    fCurrentStanzaNumber =
      elt->getAttributeValue ("number");

    if (fCurrentStanzaNumber.size () == 0) {
      msrMusicXMLWarning (
        gOahOah->fInputSourceName,
        inputLineNumber,
        "lyric number is empty, using \"1\" by default");

      fCurrentStanzaNumber = "1";
    }

    else {
#ifdef TRACE_OAH
      if (gTraceOah->fTraceLyrics) {
        fLogOutputStream <<
          "--> setting fCurrentStanzaNumber to " <<
          fCurrentStanzaNumber <<
          ", line " << inputLineNumber <<
          endl;
      }
#endif

      // register it as current stanza number, JMI
      // that remains set until another positive value is found,
      // thus allowing a skip syllable to be generated
      // for notes without lyrics
    }
  }

  // name

  {
    fCurrentStanzaName =
      elt->getAttributeValue ("name");

    if (fCurrentStanzaName.size () == 0) {
#ifdef TRACE_OAH
      if (gTraceOah->fTraceLyrics) {
        // lyrics names are not so frequent after all...
        stringstream s;

        s <<
          "lyric name is empty, using \"" <<
          K_NO_STANZA_NAME <<
          "\" by default";

        msrMusicXMLWarning (
          gOahOah->fInputSourceName,
          inputLineNumber,
          s.str ());
      }
#endif

      fCurrentStanzaName = K_NO_STANZA_NAME;
    }

    else {
#ifdef TRACE_OAH
      if (gTraceOah->fTraceLyrics) {
        fLogOutputStream <<
          "--> setting fCurrentStanzaName to " <<
          fCurrentStanzaName <<
          ", line " << inputLineNumber <<
          endl;
      }
#endif

      // register it as current stanza name, JMI
      // that remains set another positive value is found,
      // thus allowing a skip syllable to be generated
      // for notes without lyrics
    }
  }
}

void mxmlTree2MsrSkeletonBuilder::visitEnd ( S_lyric& elt )
{
  int inputLineNumber =
    elt->getInputLineNumber ();

  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> End visiting S_lyric" <<
      ", line " << inputLineNumber <<
      endl;
  }

#ifdef TRACE_OAH
  if (gTraceOah->fTraceLyrics) {
    fLogOutputStream <<
      endl <<
      "visitEnd ( S_lyric& )" <<
      ", line = " << inputLineNumber <<
      ", with:" <<
      endl;

    gIndenter++;

    fLogOutputStream <<
      "Lyric data:" <<
      endl;

    {
      gIndenter++;

      const int fieldWidth = 28;

      fLogOutputStream << left <<
        setw (fieldWidth) <<
        "fCurrentStanzaNumber" << " = " << fCurrentStanzaNumber <<
        endl;

      gIndenter--;
    }

    gIndenter--;
  }
#endif

  if (! gMsrOah->fOmitLyrics) {
    // create current voice if need be
    S_msrVoice
      currentVoice =
        createRegularVoiceInStaffIfNotYetDone (
          inputLineNumber,
          fCurrentStaffMusicXMLNumber,
          fCurrentVoiceMusicXMLNumber);

    // create stanzaNumber in current voice if need be
    S_msrStanza
      stanza =
        currentVoice->
          createStanzaInVoiceIfNotYetDone (
            inputLineNumber,
            fCurrentStanzaNumber,
            fCurrentStanzaName);
  }
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_harmony& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_harmony" <<
      ", harmonyVoicesCounter = " << fHarmonyVoicesCounter <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  /*
    several harmonies can be attached to a given note,
    leading to as many harmony voices in the current part
  */

  // take harmony voice into account
  fHarmonyVoicesCounter++; // NOT USED JMI

  fThereAreHarmoniesToBeAttachedToCurrentNote = true;
}

//______________________________________________________________________________
void mxmlTree2MsrSkeletonBuilder::visitStart ( S_figured_bass& elt )
{
  if (gMxmlTreeOah->fTraceMusicXMLTreeVisitors) {
    fLogOutputStream <<
      "--> Start visiting S_figured_bass" <<
      ", figuredBassVoicesCounter = " << fFiguredBassVoicesCounter <<
      ", line " << elt->getInputLineNumber () <<
      endl;
  }

  /*
    several figured basses can be attached to a given note,
    leading to as many figured bass voices in the current part JMI TRUE???
  */

  // take figured bass voice into account
  fFiguredBassVoicesCounter++;

  fThereAreFiguredBassToBeAttachedToCurrentNote = true;
}


} // namespace
