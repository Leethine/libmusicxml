/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include "lpsrStaves.h"

#include "lpsrVoices.h"
#include "lpsrLyrics.h"

#include "lpsrOah.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_lpsrNewStaffgroupBlock lpsrNewStaffgroupBlock::create (
  int inputLineNumber)
{
  lpsrNewStaffgroupBlock* o =
    new lpsrNewStaffgroupBlock (
      inputLineNumber);
  assert(o!=0);
  return o;
}

lpsrNewStaffgroupBlock::lpsrNewStaffgroupBlock (
  int inputLineNumber)
    : lpsrElement (inputLineNumber)
{}

lpsrNewStaffgroupBlock::~lpsrNewStaffgroupBlock ()
{}

void lpsrNewStaffgroupBlock::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrNewStaffgroupBlock::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrNewStaffgroupBlock>*
    p =
      dynamic_cast<visitor<S_lpsrNewStaffgroupBlock>*> (v)) {
        S_lpsrNewStaffgroupBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrNewStaffgroupBlock::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void lpsrNewStaffgroupBlock::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrNewStaffgroupBlock::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrNewStaffgroupBlock>*
    p =
      dynamic_cast<visitor<S_lpsrNewStaffgroupBlock>*> (v)) {
        S_lpsrNewStaffgroupBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrNewStaffgroupBlock::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void lpsrNewStaffgroupBlock::browseData (basevisitor* v)
{}

void lpsrNewStaffgroupBlock::print (ostream& os) const
{
  os << "NewStaffgroupBlock" << endl;

  gIndenter++;

  int size = fNewStaffgroupElements.size ();

  for (int i = 0; i < size; i++ ) {
    os << fNewStaffgroupElements[i];
  } // for

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_lpsrNewStaffgroupBlock& nstf)
{
  nstf->print (os);
  return os;
}

//______________________________________________________________________________
S_lpsrNewStaffTuningBlock lpsrNewStaffTuningBlock::create (
  int              inputLineNumber,
  S_msrStaffTuning staffTuning)
{
  lpsrNewStaffTuningBlock* o =
    new lpsrNewStaffTuningBlock (
      inputLineNumber, staffTuning);
  assert(o!=0);
  return o;
}

lpsrNewStaffTuningBlock::lpsrNewStaffTuningBlock (
  int              inputLineNumber,
  S_msrStaffTuning staffTuning)
    : lpsrElement (inputLineNumber)
{
  fStaffTuning = staffTuning;
}

lpsrNewStaffTuningBlock::~lpsrNewStaffTuningBlock ()
{}

void lpsrNewStaffTuningBlock::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrNewStaffTuningBlock::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrNewStaffTuningBlock>*
    p =
      dynamic_cast<visitor<S_lpsrNewStaffTuningBlock>*> (v)) {
        S_lpsrNewStaffTuningBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrNewStaffTuningBlock::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void lpsrNewStaffTuningBlock::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrNewStaffTuningBlock::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrNewStaffTuningBlock>*
    p =
      dynamic_cast<visitor<S_lpsrNewStaffTuningBlock>*> (v)) {
        S_lpsrNewStaffTuningBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrNewStaffTuningBlock::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void lpsrNewStaffTuningBlock::browseData (basevisitor* v)
{}

void lpsrNewStaffTuningBlock::print (ostream& os) const
{
  os << "NewStaffTuningBlock" << endl;

  gIndenter++;

  os <<
    fStaffTuning;

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_lpsrNewStaffTuningBlock& nstf)
{
  nstf->print (os);
  return os;
}

//______________________________________________________________________________
S_lpsrNewStaffBlock lpsrNewStaffBlock::create (
  int inputLineNumber)
{
  lpsrNewStaffBlock* o =
    new lpsrNewStaffBlock (
      inputLineNumber);
  assert(o!=0);
  return o;
}

lpsrNewStaffBlock::lpsrNewStaffBlock (
  int inputLineNumber)
    : lpsrElement (inputLineNumber)
{}

lpsrNewStaffBlock::~lpsrNewStaffBlock ()
{}

void lpsrNewStaffBlock::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrNewStaffBlock::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrNewStaffBlock>*
    p =
      dynamic_cast<visitor<S_lpsrNewStaffBlock>*> (v)) {
        S_lpsrNewStaffBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrNewStaffBlock::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void lpsrNewStaffBlock::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrNewStaffBlock::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrNewStaffBlock>*
    p =
      dynamic_cast<visitor<S_lpsrNewStaffBlock>*> (v)) {
        S_lpsrNewStaffBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrNewStaffBlock::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void lpsrNewStaffBlock::browseData (basevisitor* v)
{}

void lpsrNewStaffBlock::print (ostream& os) const
{
  os << "NewStaffBlock" << endl;

  gIndenter++;

  int size = fNewStaffElements.size ();

  for (int i = 0; i < size; i++ ) {
    os << fNewStaffElements[i];
  } // for

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_lpsrNewStaffBlock& nstf)
{
  nstf->print (os);
  return os;
}

//______________________________________________________________________________
S_lpsrStaffBlock lpsrStaffBlock::create (
  S_msrStaff staff)
{
  lpsrStaffBlock* o = new lpsrStaffBlock (
    staff);
  assert(o!=0);
  return o;
}

lpsrStaffBlock::lpsrStaffBlock (
  S_msrStaff staff)
    : lpsrElement (0) // JMI
{
  // sanity check
  msgAssert (
    staff != nullptr,
    "staff is null");

  fStaff = staff;

  // set staff block instrument names
  fStaffBlockInstrumentName =
    fStaff->
      getStaffInstrumentName ();
  fStaffBlockShortInstrumentName =
    fStaff->
      getStaffInstrumentAbbreviation ();
}

lpsrStaffBlock::~lpsrStaffBlock ()
{}

void lpsrStaffBlock::appendVoiceUseToStaffBlock (S_msrVoice voice)
{
  S_lpsrUseVoiceCommand
    useVoiceCommand =
      lpsrUseVoiceCommand::create (
        fInputLineNumber,
        voice);

  fStaffBlockElements.push_back (useVoiceCommand);
}

void lpsrStaffBlock::appendLyricsUseToStaffBlock (S_msrStanza stanza)
{
  S_lpsrNewLyricsBlock
    newLyricsCommand =
      lpsrNewLyricsBlock::create (
        fInputLineNumber,
        stanza,
        stanza->getStanzaVoiceUpLink ());

  fStaffBlockElements.push_back (newLyricsCommand);
}

void lpsrStaffBlock::acceptIn (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrStaffBlock::acceptIn ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrStaffBlock>*
    p =
      dynamic_cast<visitor<S_lpsrStaffBlock>*> (v)) {
        S_lpsrStaffBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrStaffBlock::visitStart ()" <<
            endl;
        }
#endif
        p->visitStart (elem);
  }
}

void lpsrStaffBlock::acceptOut (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrStaffBlock::acceptOut ()" <<
      endl;
  }
#endif

  if (visitor<S_lpsrStaffBlock>*
    p =
      dynamic_cast<visitor<S_lpsrStaffBlock>*> (v)) {
        S_lpsrStaffBlock elem = this;

#ifdef TRACE_OAH
        if (globalLpsrOah->fTraceLpsrVisitors) {
          gLogOstream <<
            "% ==> Launching lpsrStaffBlock::visitEnd ()" <<
            endl;
        }
#endif
        p->visitEnd (elem);
  }
}

void lpsrStaffBlock::browseData (basevisitor* v)
{
#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% ==> lpsrStaffBlock::browseData ()" <<
      endl;
  }
#endif

  for (
    list<S_msrElement>::const_iterator i = fStaffBlockElements.begin ();
    i != fStaffBlockElements.end ();
    i++
  ) {
    // browse the element
    msrBrowser<msrElement> browser (v);
    browser.browse (*(*i));
  } // for

#ifdef TRACE_OAH
  if (globalLpsrOah->fTraceLpsrVisitors) {
    gLogOstream <<
      "% <== lpsrStaffBlock::browseData ()" <<
      endl;
  }
#endif
}

void lpsrStaffBlock::print (ostream& os) const
{
  os <<
    "StaffBlock for staff \"" <<
    fStaff->getStaffName () <<
    "\" (" << fStaff->staffKindAsString () <<
    "), " <<
    singularOrPlural (
      fStaffBlockElements.size (), "element", "elements") <<
    endl;

  gIndenter++;

  const int fieldWidth = 31;

  os << left <<
    setw (fieldWidth) <<
    "(StaffBlockInstrumentName" << " = \"" << fStaffBlockInstrumentName <<
    "\")" <<
    endl <<
    setw (fieldWidth) <<
    "(StaffBlockShortInstrumentName" << " = \"" << fStaffBlockShortInstrumentName <<
    "\")" <<
    endl <<
    endl;

  if (fStaffBlockElements.size ()) {
    list<S_msrElement>::const_iterator
      iBegin = fStaffBlockElements.begin (),
      iEnd   = fStaffBlockElements.end (),
      i      = iBegin;
    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
 // JMI     os << endl;
    } // for
  }

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_lpsrStaffBlock& scr)
{
  scr->print (os);
  return os;
}


}
