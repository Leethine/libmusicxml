/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include <iostream>
#include <sstream>
#include <iomanip>      // setw, setprecision, ...

#include "utilities.h"
#include "messagesHandling.h"

#include "msrTempos.h"

#include "generalOah.h"

#include "setTraceOahIfDesired.h"
#ifdef TRACE_OAH
  #include "traceOah.h"
#endif

#include "msrOah.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_msrTempoNote msrTempoNote::create (
  int      inputLineNumber,
  rational tempoNoteWholeNotes,
  bool     tempoNoteBelongsToATuplet)
{
  msrTempoNote * o =
    new msrTempoNote (
      inputLineNumber,
      tempoNoteWholeNotes,
      tempoNoteBelongsToATuplet);
  assert(o!=0);

  return o;
}

msrTempoNote::msrTempoNote (
  int      inputLineNumber,
  rational tempoNoteWholeNotes,
  bool     tempoNoteBelongsToATuplet)
    : msrElement (inputLineNumber)
{
  fTempoNoteWholeNotes = tempoNoteWholeNotes;

  fTempoNoteBelongsToATuplet = tempoNoteBelongsToATuplet;
}

msrTempoNote::~msrTempoNote ()
{}

void msrTempoNote::appendBeamToTempoNote (S_msrBeam beam)
{
  fTempoNoteBeams.push_back (beam);
}

void msrTempoNote::acceptIn (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempoNote::acceptIn ()" <<
      endl;
  }

  if (visitor<S_msrTempoNote>*
    p =
      dynamic_cast<visitor<S_msrTempoNote>*> (v)) {
        S_msrTempoNote elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempoNote::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void msrTempoNote::acceptOut (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempoNote::acceptOut ()" <<
      endl;
  }

  if (visitor<S_msrTempoNote>*
    p =
      dynamic_cast<visitor<S_msrTempoNote>*> (v)) {
        S_msrTempoNote elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempoNote::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}

void msrTempoNote::browseData (basevisitor* v)
{
  for (
    list<S_msrBeam>::const_iterator i = fTempoNoteBeams.begin ();
    i != fTempoNoteBeams.end ();
    i++ ) {
    // browse tempoTuplet element
    msrBrowser<msrBeam> browser (v);
    browser.browse (*(*i));
  } // for
}

string msrTempoNote::asString () const
{
  stringstream s;

  s <<
    "Tempo note" <<
    ", tempoNoteWholeNotes = " << fTempoNoteWholeNotes <<
    ", tempoNoteBelongsToATuplet = " <<
    booleanAsString (fTempoNoteBelongsToATuplet);

  return s.str ();
}

void msrTempoNote::print (ostream& os) const
{
  os <<
    "TempoNote" <<
    ", line " << fInputLineNumber <<
    endl;

  gIndenter++;

  const int fieldWidth = 26;

  os << left <<
    setw (fieldWidth) <<
    "tempoNoteWholeNotes" << " : " << fTempoNoteWholeNotes <<
    endl <<
    setw (fieldWidth) <<
    "tempoNoteBelongsToATuplet" << " : " <<
    booleanAsString (fTempoNoteBelongsToATuplet) <<
    endl <<
    setw (fieldWidth) <<
    "tempoNoteBeams";

    if (fTempoNoteBeams.size ()) {
      os << endl;

      gIndenter++;

      list<S_msrBeam>::const_iterator
        iBegin = fTempoNoteBeams.begin (),
        iEnd   = fTempoNoteBeams.end (),
        i      = iBegin;
      for ( ; ; ) {
        os << (*i);
        if (++i == iEnd) break;
        os << endl;
      } // for

      gIndenter--;
    }
    else {
      os <<
        " : " <<
       "none" <<
        endl;
    }

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_msrTempoNote& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_msrTempoTuplet msrTempoTuplet::create (
  int                          inputLineNumber,
  int                          tempoTupletNumber,
  msrTempoTupletBracketKind    tempoTupletBracketKind,
  msrTempoTupletShowNumberKind tempoTupletShowNumberKind,
  msrTupletFactor              tempoTupletFactor,
  rational                     memberNotesDisplayWholeNotes)
{
  msrTempoTuplet* o =
    new msrTempoTuplet (
      inputLineNumber,
      tempoTupletNumber,
      tempoTupletBracketKind,
      tempoTupletShowNumberKind,
      tempoTupletFactor,
      memberNotesDisplayWholeNotes);
  assert(o!=0);
  return o;
}

msrTempoTuplet::msrTempoTuplet (
  int                          inputLineNumber,
  int                          tempoTupletNumber,
  msrTempoTupletBracketKind    tempoTupletBracketKind,
  msrTempoTupletShowNumberKind tempoTupletShowNumberKind,
  msrTupletFactor              tempoTupletFactor,
  rational                     memberNotesDisplayWholeNotes)
    : msrElement (inputLineNumber)
{
  fTempoTupletNumber = tempoTupletNumber;

  fTempoTupletBracketKind    = tempoTupletBracketKind;
  fTempoTupletShowNumberKind = tempoTupletShowNumberKind;

  fTempoTupletFactor = tempoTupletFactor;

  fMemberNotesDisplayWholeNotes  = memberNotesDisplayWholeNotes;

  fTempoTupletDisplayWholeNotes  = rational (0, 1);

#ifdef TRACE_OAH
  if (globalTraceOah->fTraceTempos){
    gLogOstream <<
      "Creating tempo tuplet '" <<
      this->asString () <<
      "'" <<
      endl;
  }
#endif
}

msrTempoTuplet::~msrTempoTuplet ()
{}

string msrTempoTuplet::tempoTupletTypeKindAsString (
  msrTempoTupletTypeKind tempoTupletTypeKind)
{
  string result;

  switch (tempoTupletTypeKind) {
    case msrTempoTuplet::kTempoTupletTypeNone:
      result = "tempoTupletTypeNone";
      break;
    case msrTempoTuplet::kTempoTupletTypeStart:
      result = "tempoTupletTypeStart";
      break;
    case msrTempoTuplet::kTempoTupletTypeStop:
      result = "tempoTupletTypeStop";
      break;
  } // switch

  return result;
}

string msrTempoTuplet::tempoTupletBracketKindAsString (
  msrTempoTupletBracketKind tempoTupletBracketKind)
{
  string result;

  switch (tempoTupletBracketKind) {
    case msrTempoTuplet::kTempoTupletBracketYes:
      result = "tempoTupletBracketYes";
      break;
    case msrTempoTuplet::kTempoTupletBracketNo:
      result = "tempoTupletBracketNo";
      break;
  } // switch

  return result;
}

string msrTempoTuplet::tempoTupletShowNumberKindAsString (
  msrTempoTupletShowNumberKind tempoTupletShowNumberKind)
{
  string result;

  switch (tempoTupletShowNumberKind) {
    case msrTempoTuplet::kTempoTupletShowNumberActual:
      result = "tempoTupletShowNumberActual";
      break;
    case msrTempoTuplet::kTempoTupletShowNumberBoth:
      result = "tempoTupletShowNumberBoth";
      break;
    case msrTempoTuplet::kTempoTupletShowNumberNone:
      result = "tempoTupletShowNumberNone";
      break;
  } // switch

  return result;
}

void msrTempoTuplet::addTempoNoteToTempoTuplet (S_msrTempoNote tempoNote)
{
#ifdef TRACE_OAH
  if (globalTraceOah->fTraceTempos) {
    gLogOstream <<
      "Adding tempoNote '" <<
      tempoNote->asShortString () <<
      // the information is missing to display it the normal way JMI ???
      "' to tempoTuplet '" <<
      asString () <<
      "'" <<
      endl;
  }
#endif

  fTempoTupletElements.push_back (tempoNote);

/* JMI
  // register note's tempoTuplet upLink
  note->
    setNoteTempoTupletUpLink (this);

  fTupletDisplayWholeNotes += // JMI
    note->getNoteDisplayWholeNotes ();
  fTempoTupletDisplayWholeNotes.rationalise ();

    */
}

/*
void msrTempoTuplet::addTempoTupletToTempoTuplet (S_msrTempoTuplet tempoTuplet)
{
#ifdef TRACE_OAH
  if (globalTraceOah->fTraceTempos) {
    gLogOstream <<
      "Adding tempoTuplet '" <<
      tempoTuplet->asString () <<
      "' to tempoTuplet '" <<
      asString () <<
      "'" <<
      endl;
  }
#endif

  // register tempoTuplet in elements list
  fTempoTupletElements.push_back (tempoTuplet);

  // account for tempoTuplet duration
  fTempoTupletDisplayWholeNotes += // JMI
    tempoTuplet->getTempoTupletDisplayWholeNotes ();
  fTempoTupletDisplayWholeNotes.rationalise ();

    / *
  fTempoTupletDisplayWholeNotes += // JMI
    tempoTuplet->getTempoTupletDisplayWholeNotes ();
    * /

  // don't populate tempoTuplet's measure number nor position in measure,
  // this will be done in setTupletMembersMeasureNumber ()
  / * JMI
  tempoTuplet->setTempoTupletMeasureNumber (
    fTempoTupletMeasureNumber);

  // populate tempoTuplet's position in measure
  tempoTuplet->setTempoTupletPositionInMeasure (
    fTempoTupletPositionInMeasure);
    * /
}

void msrTempoTuplet::removeFirstNoteFromTempoTuplet (
  int            inputLineNumber,
  S_msrTempoNote tempoNote)
{
#ifdef TRACE_OAH
  if (globalTraceOah->fTraceTempos) {
    gLogOstream <<
      "Removing first tempoNote '" <<
      tempoNote->asShortString () <<
      "' from tempoTuplet '" <<
      asString () <<
      "'" <<
      endl;
  }
#endif

  if (fTempoTupletElements.size ()) {
    S_msrElement
      firstTempoTupletElement =
        fTempoTupletElements.front ();

    for (
      list<S_msrElement>::iterator i=fTempoTupletElements.begin ();
      i!=fTempoTupletElements.end ();
      ++i) {
      if ((*i) == tempoNote) {
        // found note, erase it
        i = fTempoTupletElements.erase (i);

        // account for note duration
        fTempoTupletDisplayWholeNotes -= // JMI
          tempoNote->getTempoNoteWholeNotes ();
        fTempoTupletDisplayWholeNotes.rationalise ();

        // don't update measure number nor position in measure: // JMI
        // they have not been set yet

        // return from function
        return;
      }
    } // for

    stringstream s;

    s <<
      "cannot remove tempoNote " <<
      tempoNote <<
      " from tempoTuplet " << asString () <<
      " since this note has not been found in fTempoTupletElements";

    msrInternalError (
      globalOahOah->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }

  else {
    stringstream s;

    s <<
      "cannot remove tempoNote " <<
      tempoNote <<
      " from empty tempoTuplet " <<
      " since this note cannot be found in empty fTempoTupletElements";

    msrInternalError (
      globalOahOah->fInputSourceName,
      inputLineNumber,
      __FILE__, __LINE__,
      s.str ());
  }
}
*/

/* JMI
void msrTempoTuplet::applyDisplayFactorToTempoTupletMembers ()
{
#ifdef TRACE_OAH
  if (globalTraceOah->fTraceTempos) {
    gLogOstream <<
      "% ==> applyDisplayFactorToTempoTupletMembers ()" <<
      endl;

    gIndenter++;

    gLogOstream <<
      "% fTempoTupletFactor = " <<
      fTempoTupletFactor <<
      endl <<
      endl;

    gIndenter--;
  }
#endif
}

void msrTempoTuplet::unapplySoundingFactorToTempoTupletMembers (
  int containingTempoTupletActualNotes,
  int containingTempoTupletNormalNotes)
{
#ifdef TRACE_OAH
  if (globalTraceOah->fTraceTempos) {
    gLogOstream <<
      "unapplySoundingFactorToTempoTupletMembers ()" <<
      endl;

    gIndenter++;

    gLogOstream <<
      "% fTempoTupletFactor = " <<
      fTempoTupletFactor <<
      endl <<
      "% containingTempoTupletActualNotes = " <<
      containingTempoTupletActualNotes <<
      ", containingTempoTupletNormalNotes = " <<
      containingTempoTupletNormalNotes <<
      endl <<
      endl;

    gIndenter--;
  }
#endif

  fTempoTupletFactor /= // JMI ???
    containingTempoTupletActualNotes;
}
*/

void msrTempoTuplet::acceptIn (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempoTuplet::acceptIn ()" <<
      endl;
  }

  if (visitor<S_msrTempoTuplet>*
    p =
      dynamic_cast<visitor<S_msrTempoTuplet>*> (v)) {
        S_msrTempoTuplet elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempoTuplet::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void msrTempoTuplet::acceptOut (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempoTuplet::acceptOut ()" <<
      endl;
  }

  if (visitor<S_msrTempoTuplet>*
    p =
      dynamic_cast<visitor<S_msrTempoTuplet>*> (v)) {
        S_msrTempoTuplet elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempoTuplet::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}

void msrTempoTuplet::browseData (basevisitor* v)
{
  for (
    list<S_msrElement>::const_iterator i = fTempoTupletElements.begin ();
    i != fTempoTupletElements.end ();
    i++ ) {
    // browse tempoTuplet element
    msrBrowser<msrElement> browser (v);
    browser.browse (*(*i));
  } // for
}

string msrTempoTuplet::asString () const
{
  stringstream s;

  s <<
    "TempoTuplet " <<
    fTempoTupletFactor <<
    " " << fTempoTupletDisplayWholeNotes << " display whole notes" <<
    ":";

  s << "[[";

  if (fTempoTupletElements.size ()) {
    list<S_msrElement>::const_iterator
      iBegin = fTempoTupletElements.begin (),
      iEnd   = fTempoTupletElements.end (),
      i      = iBegin;
    for ( ; ; ) {

      if (
        S_msrTempoNote note = dynamic_cast<msrTempoNote*>(&(*(*i)))
        ) {
        s <<
          note->asShortString ();
      }

      else if (
        S_msrTempoTuplet tempoTuplet = dynamic_cast<msrTempoTuplet*>(&(*(*i)))
        ) {
        s <<
          tempoTuplet->asString ();
      }

      else {
        msrInternalError (
          globalOahOah->fInputSourceName,
          fInputLineNumber,
          __FILE__, __LINE__,
          "tempoTuplet member should be a note, a chord or another tempoTuplet");
      }

      if (++i == iEnd) break;
      s << " ";

    } // for
  }

  s << "]]";

  return s.str ();
}

/* JMI
string msrTempoTuplet::asString () const
{
  stringstream s;

  s <<
    "TempoTuplet " <<
    fTempoTupletFactor <<
    " " << fTempoTupletSoundingWholeNotes << " sound whole notes" <<
    " measure '"<<
    fTempoTupletMeasureNumber <<
    "':";

  if (fTempoTupletPositionInMeasure.getNumerator () < 0) {
    s << "?";
  }
  else {
    s << fTempoTupletPositionInMeasure;
  }

  s << "[[";

  if (fTempoTupletElements.size ()) {
    list<S_msrElement>::const_iterator
      iBegin = fTempoTupletElements.begin (),
      iEnd   = fTempoTupletElements.end (),
      i      = iBegin;
    for ( ; ; ) {

      if (
        S_msrTempoNote note = dynamic_cast<msrTempoNote*>(&(*(*i)))
        ) {
        s <<
          note->asShortString ();
      }

      else if (
        S_msrChord chord = dynamic_cast<msrChord*>(&(*(*i)))
        ) {
        s <<
          chord->asString ();
      }

      else if (
        S_msrTempoTuplet tempoTuplet = dynamic_cast<msrTempoTuplet*>(&(*(*i)))
        ) {
        s <<
          tempoTuplet->asString ();
      }

      else {
        msrInternalError (
          globalOahOah->fInputSourceName,
          fInputLineNumber,
          __FILE__, __LINE__,
          "tempoTuplet member should be a note, a chord or another tempoTuplet");
      }

      if (++i == iEnd) break;
      s << " ";
    } // for
  }

  s << "]]";

  return s.str ();
}
*/

void msrTempoTuplet::print (ostream& os) const
{
  os <<
    "TempoTuplet " <<
    fTempoTupletFactor <<
    ", " <<
    singularOrPlural (
      fTempoTupletElements.size (), "element", "elements") <<
    ", display whole notes: " <<
    fTempoTupletDisplayWholeNotes <<
    ", line " << fInputLineNumber <<
    endl;

  gIndenter++;

  const int fieldWidth = 30;

  os << left <<
    setw (fieldWidth) <<
    "TempoTupletBracketKind" << " : " <<
    tempoTupletBracketKindAsString (
      fTempoTupletBracketKind) <<
    endl <<
    setw (fieldWidth) <<
    "TempoTupletShowNumberKind" << " : " <<
    tempoTupletShowNumberKindAsString (
      fTempoTupletShowNumberKind) <<
    endl <<
    setw (fieldWidth) <<
    "MemberNotesDisplayWholeNotes" << " : " <<
    fMemberNotesDisplayWholeNotes <<
    endl <<
    endl;

/* JMI ???
  os << left <<
    setw (fieldWidth) <<
    "(position in measure" << " : ";
  if (fTempoTupletPositionInMeasure.getNumerator () < 0) {
    os << "???)";
  }
  else {
    os << fTempoTupletPositionInMeasure << ")";
  }
  os << endl;
    */

  os <<
    "TempoTupletsElements";

  if (fTempoTupletElements.size ()) {
    gIndenter++;

    list<S_msrElement>::const_iterator
      iBegin = fTempoTupletElements.begin (),
      iEnd   = fTempoTupletElements.end (),
      i      = iBegin;
    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
      // no endl here
    } // for

    gIndenter--;
    }
  else {
    os << left <<
      " : " << "none" <<
      endl;
  }

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_msrTempoTuplet& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_msrTempoRelationshipElements msrTempoRelationshipElements::create (
  int      inputLineNumber,
  msrTempoRelationshipElementsKind
           tempoRelationshipElementsKind)
{
  msrTempoRelationshipElements * o =
    new msrTempoRelationshipElements (
      inputLineNumber,
      tempoRelationshipElementsKind);
  assert(o!=0);

  return o;
}

msrTempoRelationshipElements::msrTempoRelationshipElements (
  int      inputLineNumber,
  msrTempoRelationshipElementsKind
           tempoRelationshipElementsKind)
    : msrElement (inputLineNumber)
{
  fTempoRelationshipElementsKind = tempoRelationshipElementsKind;
}

msrTempoRelationshipElements::~msrTempoRelationshipElements ()
{}

void msrTempoRelationshipElements::addElementToTempoRelationshipElements (
  S_msrElement element)
{
#ifdef TRACE_OAH
  if (globalTraceOah->fTraceTempos){
    gLogOstream <<
      "Adding element '" <<
      element->asString () <<
      "' to tempo relationship" <<
      endl;
  }
#endif

  fTempoRelationshipElementsList.push_back (element);
}

void msrTempoRelationshipElements::acceptIn (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempoRelationshipElements::acceptIn ()" <<
      endl;
  }

  if (visitor<S_msrTempoRelationshipElements>*
    p =
      dynamic_cast<visitor<S_msrTempoRelationshipElements>*> (v)) {
        S_msrTempoRelationshipElements elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempoRelationshipElements::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void msrTempoRelationshipElements::acceptOut (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempoRelationshipElements::acceptOut ()" <<
      endl;
  }

  if (visitor<S_msrTempoRelationshipElements>*
    p =
      dynamic_cast<visitor<S_msrTempoRelationshipElements>*> (v)) {
        S_msrTempoRelationshipElements elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempoRelationshipElements::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}

void msrTempoRelationshipElements::browseData (basevisitor* v)
{
  // browse the elements list
  if (fTempoRelationshipElementsList.size ()) {
    for (
      list<S_msrElement>::const_iterator i =
        fTempoRelationshipElementsList.begin ();
      i != fTempoRelationshipElementsList.end ();
      i++
  ) {
      // browse the element
      msrBrowser<msrElement> browser (v);
      browser.browse (*(*i));
    } // for
  }
}

string msrTempoRelationshipElements::asString () const
{
  stringstream s;

  s <<
    "TempoRelationshipElements" <<
    ", fTempoRelationshipElementsKind = " << fTempoRelationshipElementsKind;

  return s.str ();
}

string msrTempoRelationshipElements::tempoRelationshipElementsKindAsString (
  msrTempoRelationshipElementsKind tempoRelationshipElementsKind)
{
  string result;

  switch (tempoRelationshipElementsKind) {
    case msrTempoRelationshipElements::kTempoRelationshipElementsLeft:
      result = "tempoRelationshipElementsLeft";
      break;
    case msrTempoRelationshipElements::kTempoRelationshipElementsRight:
      result = "tempoRelationshipElementsRight";
      break;
  } // switch

  return result;
}

void msrTempoRelationshipElements::print (ostream& os) const
{
  os <<
    "TempoRelationshipElements" <<
    ", line " << fInputLineNumber <<
    endl;

  gIndenter++;

  os <<
    "tempoRelationshipElementsKindAsString : " <<
    tempoRelationshipElementsKindAsString (
      fTempoRelationshipElementsKind) <<
    endl;

  const int fieldWidth = 26;

  os << left <<
    setw (fieldWidth) <<
    "tempoRelationshipElementsList";

    if (fTempoRelationshipElementsList.size ()) {
      gIndenter++;

      os << endl;

      list<S_msrElement>::const_iterator
        iBegin = fTempoRelationshipElementsList.begin (),
        iEnd   = fTempoRelationshipElementsList.end (),
        i      = iBegin;

      for ( ; ; ) {
        os << (*i);
        if (++i == iEnd) break;
        // os << endl;
      } // for

      gIndenter--;
  }
  else {
    os <<
      " : " << "none ???" <<
      endl;
  }

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_msrTempoRelationshipElements& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_msrTempo msrTempo::create (
  int               inputLineNumber,
  S_msrWords        tempoWords)
{
  msrTempo* o =
    new msrTempo (
      inputLineNumber,
      tempoWords);
  assert(o!=0);

  return o;
}

S_msrTempo msrTempo::create (
  int               inputLineNumber,
  msrDottedDuration tempoBeatUnit,
  string            tempoPerMinute,
  msrTempoParenthesizedKind
                    tempoParenthesizedKind,
  msrPlacementKind  tempoPlacementKind)
{
  msrTempo* o =
    new msrTempo (
      inputLineNumber,
      tempoBeatUnit,
      tempoPerMinute,
      tempoParenthesizedKind,
      tempoPlacementKind);
  assert(o!=0);
  return o;
}

S_msrTempo msrTempo::create (
  int               inputLineNumber,
  msrDottedDuration tempoBeatUnit,
  msrDottedDuration tempoEquivalentBeatUnit,
  msrTempoParenthesizedKind
                    tempoParenthesizedKind,
  msrPlacementKind  tempoPlacementKind)
{
  msrTempo* o =
    new msrTempo (
      inputLineNumber,
      tempoBeatUnit,
      tempoEquivalentBeatUnit,
      tempoParenthesizedKind,
      tempoPlacementKind);
  assert(o!=0);
  return o;
}

S_msrTempo msrTempo::create (
  int               inputLineNumber,
  S_msrTempoRelationshipElements
                    tempoRelationLeftElements,
  S_msrTempoRelationshipElements
                    tempoRelationRightElements,
  msrTempoParenthesizedKind
                    tempoParenthesizedKind,
  msrPlacementKind  tempoPlacementKind)
{
  msrTempo* o =
    new msrTempo (
      inputLineNumber,
      tempoRelationLeftElements,
      tempoRelationRightElements,
      tempoParenthesizedKind,
      tempoPlacementKind);
  assert(o!=0);
  return o;
}

msrTempo::msrTempo (
  int               inputLineNumber,
  S_msrWords        tempoWords)
    : msrMeasureElement (inputLineNumber),
      fTempoBeatUnit (),
      fTempoEquivalentBeatUnit ()
{
  fTempoKind = kTempoBeatUnitsWordsOnly;

  fTempoPerMinute = "";

  fTempoParenthesizedKind = kTempoParenthesizedNo;

  fTempoPlacementKind = kPlacementAbove;

  fTempoWordsList.push_back (tempoWords);
}

msrTempo::msrTempo (
  int               inputLineNumber,
  msrDottedDuration tempoBeatUnit,
  string            tempoPerMinute,
  msrTempoParenthesizedKind
                    tempoParenthesizedKind,
  msrPlacementKind  tempoPlacementKind)
    : msrMeasureElement (inputLineNumber),
      fTempoBeatUnit (tempoBeatUnit),
      fTempoEquivalentBeatUnit ()
{
  fTempoKind = kTempoBeatUnitsPerMinute;

  fTempoPerMinute = tempoPerMinute;

  fTempoParenthesizedKind = tempoParenthesizedKind;

  fTempoPlacementKind = tempoPlacementKind;
}

msrTempo::msrTempo (
  int               inputLineNumber,
  msrDottedDuration tempoBeatUnit,
  msrDottedDuration tempoEquivalentBeatUnit,
  msrTempoParenthesizedKind
                    tempoParenthesizedKind,
  msrPlacementKind  tempoPlacementKind)
    : msrMeasureElement (inputLineNumber),
      fTempoBeatUnit (tempoBeatUnit),
      fTempoEquivalentBeatUnit (tempoEquivalentBeatUnit)
{
  fTempoKind = kTempoBeatUnitsEquivalence;

  fTempoPerMinute = "";

  fTempoParenthesizedKind = tempoParenthesizedKind;

  fTempoPlacementKind = tempoPlacementKind;
}

msrTempo::msrTempo (
  int               inputLineNumber,
  S_msrTempoRelationshipElements
                    tempoRelationLeftElements,
  S_msrTempoRelationshipElements
                    tempoRelationRightElements,
  msrTempoParenthesizedKind
                    tempoParenthesizedKind,
  msrPlacementKind  tempoPlacementKind)
    : msrMeasureElement (inputLineNumber)
{
  fTempoKind = kTempoNotesRelationShip;

  fTempoPerMinute = "";

  fTempoRelationLeftElements =
    tempoRelationLeftElements;
  fTempoRelationRightElements =
    tempoRelationRightElements;

  fTempoParenthesizedKind = tempoParenthesizedKind;

  fTempoPlacementKind = tempoPlacementKind;
}

msrTempo::~msrTempo ()
{}

void msrTempo::acceptIn (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempo::acceptIn ()" <<
      endl;
  }

  if (visitor<S_msrTempo>*
    p =
      dynamic_cast<visitor<S_msrTempo>*> (v)) {
        S_msrTempo elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempo::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void msrTempo::acceptOut (basevisitor* v)
{
  if (globalMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrTempo::acceptOut ()" <<
      endl;
  }

  if (visitor<S_msrTempo>*
    p =
      dynamic_cast<visitor<S_msrTempo>*> (v)) {
        S_msrTempo elem = this;

        if (globalMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrTempo::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}


void msrTempo::browseData (basevisitor* v)
{
  switch (fTempoKind) {
    case msrTempo::k_NoTempoKind:
      break;

    case msrTempo::kTempoBeatUnitsWordsOnly:
      break;

    case msrTempo::kTempoBeatUnitsPerMinute:
      break;

    case msrTempo::kTempoBeatUnitsEquivalence:
      break;

    case msrTempo::kTempoNotesRelationShip:
      {
        // browse the left elements
        if (fTempoRelationLeftElements) {
          msrBrowser<msrTempoRelationshipElements> browser (v);
          browser.browse (*fTempoRelationLeftElements);
        }

        // browse the right elements
        if (fTempoRelationRightElements) {
          msrBrowser<msrTempoRelationshipElements> browser (v);
          browser.browse (*fTempoRelationRightElements);
        }
      }
      break;
  } // switch
}

string msrTempo::tempoKindAsString (
  msrTempoKind tempoKind)
{
  string result;

  switch (tempoKind) {
    case msrTempo::k_NoTempoKind:
      result = "noTempoKind???";
      break;
    case msrTempo::kTempoBeatUnitsWordsOnly:
      result = "tempoBeatUnitsWordsOnly";
      break;
    case msrTempo::kTempoBeatUnitsPerMinute:
      result = "tempoBeatUnitsPerMinute";
      break;
    case msrTempo::kTempoBeatUnitsEquivalence:
      result = "tempoBeatUnitsEquivalence";
      break;
    case msrTempo::kTempoNotesRelationShip:
      result = "tempoNotesRelationShip";
      break;
  } // switch

  return result;
}

string msrTempo::tempoParenthesizedKindAsString (
  msrTempoParenthesizedKind tempoParenthesizedKind)
{
  string result;

  switch (tempoParenthesizedKind) {
    case msrTempo::kTempoParenthesizedYes:
      result = "tempoParenthesizedYes";
      break;
    case msrTempo::kTempoParenthesizedNo:
      result = "tempoParenthesizedNo";
      break;
  } // switch

  return result;
}

string msrTempo::tempoRelationKindAsString (
  msrTempoRelationKind tempoRelationKind)
{
  string result;

  switch (tempoRelationKind) {
    case msrTempo::kTempoRelationEquals:
      result = "tempoRelationEquals";
      break;
    case msrTempo::kTempoRelationNone:
      result = "tempoRelationNone";
      break;
  } // switch

  return result;
}

string msrTempo::tempoWordsListAsString (string separator) const
{
  string result;

  list<S_msrWords>::const_iterator
    iBegin = fTempoWordsList.begin (),
    iEnd   = fTempoWordsList.end (),
    i      = iBegin;
  for ( ; ; ) {
    result += (*i)->getWordsContents ();
    if (++i == iEnd) break;
    result += separator;
  } // for

  return result;
}

string msrTempo::asString () const
{
  stringstream s;

  s <<
    "Tempo" <<
    ", tempoKind: " << tempoKindAsString (fTempoKind) <<
    ", tempoWordsList: ";

  if (fTempoWordsList.size ()) {
    list<S_msrWords>::const_iterator
      iBegin = fTempoWordsList.begin (),
      iEnd   = fTempoWordsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      s << (*i);
      if (++i == iEnd) break;
      s << ", ";
    } // for
  }
  else {
    s << "\"\"";
  }

  s <<
    ", tempoBeatUnit: " << fTempoBeatUnit.asString () <<
    ", tempoPerMinute: " << fTempoPerMinute <<
    ", tempoParenthesizedKind: "  <<
    tempoParenthesizedKindAsString (fTempoParenthesizedKind) <<
    ", line " << fInputLineNumber;

  return s.str ();
}

void msrTempo::print (ostream& os) const
{
  os <<
    "Tempo" <<
    ", line " << fInputLineNumber <<
    endl;

  gIndenter++;

  const int fieldWidth = 27;

  os << left <<
    setw (fieldWidth) <<
   "tempoKind" << " : " << tempoKindAsString (fTempoKind) <<
    endl;

  os << left <<
    setw (fieldWidth) <<
    "tempoWords";

  if (fTempoWordsList.size ()) {
    os << endl;

    gIndenter++;

    list<S_msrWords>::const_iterator
      iBegin = fTempoWordsList.begin (),
      iEnd   = fTempoWordsList.end (),
      i      = iBegin;
    for ( ; ; ) {
      os << (*i);
      if (++i == iEnd) break;
      os << endl;
    } // for

    gIndenter--;
  }
  else {
    os <<
      " : " << "none" <<
      endl;;
  }

  os << left <<
    setw (fieldWidth) <<
    "tempoBeatUnit :" <<
    endl;

  gIndenter++;

  os <<
    fTempoBeatUnit;

  gIndenter--;

  os << left <<
    setw (fieldWidth) <<
    "tempoPerMinute" << " : \"" << fTempoPerMinute << "\"" <<
    endl <<
    setw (fieldWidth) <<
    "tempoParenthesizedKind"  << " : " <<
    tempoParenthesizedKindAsString (fTempoParenthesizedKind) <<
    endl;

  os << left <<
    setw (fieldWidth) <<
    "tempoRelationLeftElements";
  if (fTempoRelationLeftElements) {
    os <<
      " : " <<
      endl;

    gIndenter++;

    os <<
      fTempoRelationLeftElements;

    gIndenter--;
  }
  else {
    os << " : " << "none" <<
    endl;
  }

  os << left <<
    setw (fieldWidth) <<
    "tempoRelationKind" << " : " <<
    tempoRelationKindAsString (
      fTempoRelationKind) <<
    endl;


  os << left <<
    setw (fieldWidth) <<
    "tempoRelationRightElements";
  if (fTempoRelationRightElements) {
    os <<
      ":" <<
      endl;

    gIndenter++;

    os <<
      fTempoRelationRightElements;

    gIndenter--;
  }
  else {
    os << " : " << "none" <<
    endl;
  }

  gIndenter--;
}

void msrTempo::printShort (ostream& os) const
{
  os << asString () << endl;
}

ostream& operator<< (ostream& os, const S_msrTempo& elt)
{
  elt->print (os);
  return os;
}


}
