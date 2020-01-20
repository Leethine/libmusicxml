/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include "msrMutualDependencies.h"

#include "setTraceOahIfDesired.h"
#ifdef TRACE_OAH
  #include "traceOah.h"
#endif

#include "msrOah.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_msrSyllable msrSyllable::create (
  int                   inputLineNumber,
  msrSyllableKind       syllableKind,
  msrSyllableExtendKind syllableExtendKind,
  rational              syllableWholeNotes,
  msrTupletFactor       syllableTupletFactor,
  S_msrStanza           syllableStanzaUpLink)
{
  msrSyllable* o =
    new msrSyllable (
      inputLineNumber,
      syllableKind,
      syllableExtendKind,
      syllableWholeNotes,
      syllableTupletFactor,
      syllableStanzaUpLink);
  assert(o!=0);

  return o;
}

S_msrSyllable msrSyllable::createWithNextMeasurePuristNumber (
  int                   inputLineNumber,
  msrSyllableKind       syllableKind,
  msrSyllableExtendKind syllableExtendKind,
  rational              syllableWholeNotes,
  msrTupletFactor       syllableTupletFactor,
  S_msrStanza           syllableStanzaUpLink,
  int                   syllableNextMeasurePuristNumber)
{
  msrSyllable* o =
    new msrSyllable (
      inputLineNumber,
      syllableKind,
      syllableExtendKind,
      syllableWholeNotes,
      syllableTupletFactor,
      syllableStanzaUpLink,
      syllableNextMeasurePuristNumber);
  assert(o!=0);

  return o;
}

msrSyllable::msrSyllable (
  int                   inputLineNumber,
  msrSyllableKind       syllableKind,
  msrSyllableExtendKind syllableExtendKind,
  rational              syllableWholeNotes,
  msrTupletFactor       syllableTupletFactor,
  S_msrStanza           syllableStanzaUpLink)
    : msrMeasureElement (inputLineNumber)
{
  // sanity check
  msrAssert(
    syllableStanzaUpLink != nullptr,
    "syllableStanzaUpLink is null");

  // set syllable's stanza upLink
  fSyllableStanzaUpLink =
    syllableStanzaUpLink;

  fSyllableKind = syllableKind;

  fSyllableExtendKind = syllableExtendKind;

  // fSyllableNoteUpLink will be set
  // by appendSyllableToNoteAndSetItsNoteUpLink () later

  fSyllableWholeNotes = syllableWholeNotes;

  fSyllableTupletFactor = syllableTupletFactor;

  fSyllableNextMeasurePuristNumber = -1;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceLyrics) {
    gLogOstream <<
      "Creating a syllable containing:" <<
      endl;

    gIndenter++;

    print (gLogOstream);

    gIndenter--;
  }
#endif
}

msrSyllable::msrSyllable (
  int                   inputLineNumber,
  msrSyllableKind       syllableKind,
  msrSyllableExtendKind syllableExtendKind,
  rational              syllableWholeNotes,
  msrTupletFactor       syllableTupletFactor,
  S_msrStanza           syllableStanzaUpLink,
  int                   syllableNextMeasurePuristNumber)
    : msrSyllable (
        inputLineNumber,
        syllableKind,
        syllableExtendKind,
        syllableWholeNotes,
        syllableTupletFactor,
        syllableStanzaUpLink)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceLyrics) {
    gLogOstream <<
      "Setting syllable next measure purist number to " <<
      fSyllableNextMeasurePuristNumber <<
      endl;
  }
#endif

  switch (fSyllableKind) {
    case msrSyllable::kSyllableNone:
    case msrSyllable::kSyllableSingle:
    case msrSyllable::kSyllableBegin:
    case msrSyllable::kSyllableMiddle:
    case msrSyllable::kSyllableEnd:
    case msrSyllable::kSyllableOnRestNote:
    case msrSyllable::kSyllableSkipRestNote:
    case msrSyllable::kSyllableSkipNonRestNote:
    case msrSyllable::kSyllableMeasureEnd:
      msrInternalError (
        gOahOah->fInputSourceName,
        fInputLineNumber,
        __FILE__, __LINE__,
        "syllable with next measure purist number is no line nor page break");
      break;

    case msrSyllable::kSyllableLineBreak:
    case msrSyllable::kSyllablePageBreak:
      break;
  } // switch

  fSyllableNextMeasurePuristNumber = syllableNextMeasurePuristNumber;
}

msrSyllable::~msrSyllable ()
{}

S_msrSyllable msrSyllable::createSyllableNewbornClone (
  S_msrPart containingPart)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceLyrics) {
    gLogOstream <<
      "Creating a newborn clone of syllable '" <<
      asString () <<
      "'" <<
      endl;
  }
#endif

  // sanity check
  msrAssert(
    containingPart != nullptr,
    "containingPart is null");

  S_msrSyllable
    newbornClone =
      msrSyllable::create (
        fInputLineNumber,
        fSyllableKind,
        fSyllableExtendKind,
        fSyllableWholeNotes,
        fSyllableTupletFactor,
        fSyllableStanzaUpLink);

  // append the lyric texts to the syllable clone
  for (
    list<string>::const_iterator i = fSyllableTextsList.begin ();
    i!=fSyllableTextsList.end ();
    i++
  ) {
    newbornClone->
      appendLyricTextToSyllable ((*i));
  } // for

  // dont't set 'newbornClone->fSyllableStanzaUpLink'
  // nor 'newbornClone->fSyllableNoteUpLink',
  // this will be done by the caller

  newbornClone->fSyllableNoteUpLink =
    fSyllableNoteUpLink; // TEMP

  return newbornClone;
}

S_msrSyllable msrSyllable::createSyllableDeepCopy (
  S_msrPart containingPart)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceLyrics) {
    gLogOstream <<
      "Creating a newborn clone of syllable '" <<
      asString () <<
      "'" <<
      endl;
  }
#endif

  // sanity check
  msrAssert(
    containingPart != nullptr,
    "containingPart is null");

  S_msrSyllable
    syllableDeepCopy =
      msrSyllable::create (
        fInputLineNumber,
        fSyllableKind,
        fSyllableExtendKind,
        fSyllableWholeNotes,
        fSyllableTupletFactor,
        fSyllableStanzaUpLink);

  // append the lyric texts to the syllable deep copy
  for (
    list<string>::const_iterator i = fSyllableTextsList.begin ();
    i!=fSyllableTextsList.end ();
    i++
  ) {
    syllableDeepCopy->
      appendLyricTextToSyllable ((*i));
  } // for

  // dont't set 'newbornClone->fSyllableStanzaUpLink'
  // nor 'newbornClone->fSyllableNoteUpLink',
  // this will be done by the caller

  syllableDeepCopy->fSyllableNoteUpLink =
    fSyllableNoteUpLink; // TEMP

  return syllableDeepCopy;
}

void msrSyllable::appendLyricTextToSyllable (string text)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceLyrics) {
    gLogOstream <<
      "Appending text \"" <<
      text <<
      "\" to the texts list of syllable '" <<
      asString () <<
      "'" <<
      endl;
  }
#endif

  fSyllableTextsList.push_back (
    text);
}

void msrSyllable::appendSyllableToNoteAndSetItsNoteUpLink (
  S_msrNote note)
{
  // sanity check
  msrAssert (
    note != nullptr,
    "note is empty");

  fSyllableNoteUpLink = note;

/*
  // sanity check JMI ???
  msrAssert (
    fSyllableTextsList.size () != 0,
    "fSyllableTextsList is empty");
    */

  // append syllable to note
  note->
    appendSyllableToNote (this);

  // set it upLink to note
#ifdef TRACE_OAH
  if (gTraceOah->fTraceLyrics) {
    gLogOstream <<
      "Setting syllable note upLink for:" <<
      endl;

    gIndenter++;

    gLogOstream <<
      asString () <<
    // JMI    "to '" << note->asString () <<
      ", line " << note->getInputLineNumber () <<
      endl;

    gIndenter--;
  }
#endif
}

void msrSyllable::acceptIn (basevisitor* v)
{
  if (gMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrSyllable::acceptIn ()" <<
      endl;
  }

  if (visitor<S_msrSyllable>*
    p =
      dynamic_cast<visitor<S_msrSyllable>*> (v)) {
        S_msrSyllable elem = this;

        if (gMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrSyllable::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void msrSyllable::acceptOut (basevisitor* v)
{
  if (gMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrSyllable::acceptOut ()" <<
      endl;
  }

  if (visitor<S_msrSyllable>*
    p =
      dynamic_cast<visitor<S_msrSyllable>*> (v)) {
        S_msrSyllable elem = this;

        if (gMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrSyllable::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}

void msrSyllable::browseData (basevisitor* v)
{}

string msrSyllable::syllableWholeNotesAsMsrString () const
{
  string result;

  if (fSyllableNoteUpLink) { // JMI
    switch (fSyllableNoteUpLink->getNoteKind ()) {
      case msrNote::k_NoNoteKind:
      case msrNote::kRestNote:
      case msrNote::kUnpitchedNote:
      case msrNote::kStandaloneNote:
      case msrNote::kDoubleTremoloMemberNote:
      case msrNote::kGraceNote:
      case msrNote::kGraceChordMemberNote:
      case msrNote::kChordMemberNote:
        result =
          wholeNotesAsMsrString (
            fInputLineNumber,
            fSyllableWholeNotes);
        break;

      case msrNote::kSkipNote:
      case msrNote::kTupletMemberNote:
      case msrNote::kGraceTupletMemberNote:
      case msrNote::kTupletMemberUnpitchedNote:
        {
          stringstream s;

          s <<
            fSyllableNoteUpLink->
      // JMI        noteSoundingWholeNotesAsMsrString () <<
              noteDisplayWholeNotesAsMsrString () <<
            "*" <<
            fSyllableTupletFactor.getTupletNormalNotes () <<
            "/" <<
            fSyllableTupletFactor.getTupletActualNotes ();

          result = s.str ();
        }
        break;
    } // switch
  }

  else {
    result =
      wholeNotesAsMsrString (
        fInputLineNumber,
          fSyllableWholeNotes);
  }

  return result;
}

string msrSyllable::syllableKindAsString (
  msrSyllableKind syllableKind)
{
  string result;

  switch (syllableKind) {
    case msrSyllable::kSyllableNone:
      result = "syllableNone";
      break;

    case msrSyllable::kSyllableSingle:
      result = "syllableSingle";
      break;

    case msrSyllable::kSyllableBegin:
      result = "syllableBegin";
      break;
    case msrSyllable::kSyllableMiddle:
      result = "syllableMiddle";
      break;
    case msrSyllable::kSyllableEnd:
      result = "syllableEnd";
      break;

    case msrSyllable::kSyllableOnRestNote:
      result = "syllableOnRestNote";
      break;

    case msrSyllable::kSyllableSkipRestNote:
      result = "syllableSkipRestNote";
      break;
    case msrSyllable::kSyllableSkipNonRestNote:
      result = "syllableSkipNonRestNote";
      break;

    case msrSyllable::kSyllableMeasureEnd:
      result = "syllableMeasureEnd";
      break;

    case msrSyllable::kSyllableLineBreak:
      result = "syllableLineBreak";
      break;
    case msrSyllable::kSyllablePageBreak:
      result = "syllablePageBreak";
      break;
  } // switch

  return result;
}

string msrSyllable::syllableKindAsString () const
{
  return syllableKindAsString (fSyllableKind);
}

string msrSyllable::syllableExtendKindAsString (
  msrSyllableExtendKind syllableExtendKind)
{
  string result;

  switch (syllableExtendKind) {
    case msrSyllable::kSyllableExtendNone:
      result = "syllableExtendNone";
      break;
    case msrSyllable::kSyllableExtendSingle:
      result = "syllableExtendSingle";
      break;
    case msrSyllable::kSyllableExtendStart:
      result = "syllableExtendStart";
      break;
    case msrSyllable::kSyllableExtendContinue:
      result = "syllableExtendContinue";
      break;
    case msrSyllable::kSyllableExtendStop:
      result = "syllableExtendStop";
      break;
  } // switch

  return result;
}

string msrSyllable::syllableExtendKindAsString () const
{
  return syllableExtendKindAsString (fSyllableExtendKind);
}

string msrSyllable::syllableNoteUpLinkAsString () const
{
  string result;

  if (fSyllableNoteUpLink) {
    result = fSyllableNoteUpLink->asString ();
  }
  else {
    result = "none";
  }

  return result;
}

string msrSyllable::syllableTextsListAsString () const
{
  stringstream s;

  if (fSyllableTextsList.size ()) {
    list<string>::const_iterator
      iBegin = fSyllableTextsList.begin (),
      iEnd   = fSyllableTextsList.end (),
      i      = iBegin;

    for ( ; ; ) {
      s << doubleQuoteStringIfNonAlpha (*i);
      if (++i == iEnd) break;
      s << ", ";
    } // for
  }

  return s.str ();
}

void msrSyllable::writeTextsList (
  const list<string>& textsList,
  ostream&            os)
{
  os << "[";

  if (textsList.size ()) {
    list<string>::const_iterator
      iBegin = textsList.begin (),
      iEnd   = textsList.end (),
      i      = iBegin;

    for ( ; ; ) {
      os << doubleQuoteStringIfNonAlpha (*i);
      if (++i == iEnd) break;
      os << ", ";
    } // for
  }

  os << "]";
}

string msrSyllable::asString () const
{
  stringstream s;

  s <<
    "Syllable '" <<
    syllableKindAsString () <<
    "', syllableExtendKind: " <<
      syllableExtendKindAsString (fSyllableExtendKind) <<
    ", syllableWholeNotes: " << fSyllableWholeNotes <<
    ", syllableTupletFactor: " << fSyllableTupletFactor <<
    ", syllableNextMeasurePuristNumber: " << fSyllableNextMeasurePuristNumber <<
    ", line " << fInputLineNumber <<
    ", syllableTextsList: ";

  writeTextsList (
    fSyllableTextsList,
    s);

  s <<
    ", " <<
    syllableNoteUpLinkAsString ();

  switch (fSyllableKind) {
    case msrSyllable::kSyllableNone:
      msrInternalError (
        gOahOah->fInputSourceName,
        fInputLineNumber,
        __FILE__, __LINE__,
        "syllable type has not been set");
      break;

    case msrSyllable::kSyllableSingle:
    case msrSyllable::kSyllableBegin:
    case msrSyllable::kSyllableMiddle:
    case msrSyllable::kSyllableEnd:
    case msrSyllable::kSyllableOnRestNote:
    case msrSyllable::kSyllableSkipRestNote:
    case msrSyllable::kSyllableSkipNonRestNote:
    case msrSyllable::kSyllableMeasureEnd:
    case msrSyllable::kSyllableLineBreak:
    case msrSyllable::kSyllablePageBreak:
      break;
  } // switch

  s <<
    ", attached to note " <<
    fSyllableNoteUpLink->asShortString () <<
    ", in stanza " <<
    fSyllableStanzaUpLink->getStanzaName ();

  return s.str ();
}

void msrSyllable::print (ostream& os) const
{
  os <<
    "Syllable" <<
    ", syllableKind: " <<
    syllableKindAsString () <<
    ", line " << fInputLineNumber <<
    endl;

  gIndenter++;

  const int fieldWidth = 32;
  os << left <<
    setw (fieldWidth) <<
    "syllableNoteUpLink" << " : " <<
    syllableNoteUpLinkAsString () <<
    endl <<
    setw (fieldWidth) <<
    "syllableStanzaUpLink" << " : " <<
    fSyllableStanzaUpLink->getStanzaName () <<
    endl <<

    setw (fieldWidth) <<
    "syllableKind" << " : " <<
    syllableKindAsString (fSyllableKind) <<
    endl <<
    setw (fieldWidth) <<
    "syllableExtendKind" << " : " <<
    syllableExtendKindAsString (
      fSyllableExtendKind) <<
    endl <<

    setw (fieldWidth) <<
    "syllableWholeNotes" << " : " <<
    fSyllableWholeNotes <<
    endl <<

    setw (fieldWidth) <<
    "syllableTupletFactor" << " : " <<
    fSyllableTupletFactor.asString () <<
    endl <<

    setw (fieldWidth) <<
    "syllableNextMeasurePuristNumber" << " : " <<
    fSyllableNextMeasurePuristNumber <<
    endl;

  os << left <<
    setw (fieldWidth) <<
    "syllableTextsList" << " : ";

  writeTextsList (
    fSyllableTextsList,
    os);

  switch (fSyllableKind) { // JMI
    case msrSyllable::kSyllableNone:
      msrInternalError (
        gOahOah->fInputSourceName,
        fInputLineNumber,
        __FILE__, __LINE__,
        "syllable type has not been set");
      break;

    case msrSyllable::kSyllableSingle:
    case msrSyllable::kSyllableBegin:
    case msrSyllable::kSyllableMiddle:
    case msrSyllable::kSyllableEnd:
    case msrSyllable::kSyllableOnRestNote:
    case msrSyllable::kSyllableSkipRestNote:
    case msrSyllable::kSyllableSkipNonRestNote:
    case msrSyllable::kSyllableMeasureEnd:
    case msrSyllable::kSyllableLineBreak:
    case msrSyllable::kSyllablePageBreak:
      break;
  } // switch
  os << endl;

  gIndenter--;
}

ostream& operator<< (ostream& os, const S_msrSyllable& elt)
{
  elt->print (os);
  return os;
}


}
