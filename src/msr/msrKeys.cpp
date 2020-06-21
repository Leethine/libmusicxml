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

#include "utilities.h"
#include "messagesHandling.h"

#include "msrKeys.h"

#include "setTraceOahIfDesired.h"
#ifdef TRACE_OAH
  #include "traceOah.h"
#endif

#include "msrOah.h"


using namespace std;

namespace MusicXML2
{

//______________________________________________________________________________
S_msrHumdrumScotKeyItem msrHumdrumScotKeyItem::create (
  int inputLineNumber)
{
  msrHumdrumScotKeyItem* o =
    new msrHumdrumScotKeyItem (
      inputLineNumber);
  assert (o!=0);

  return o;
}

msrHumdrumScotKeyItem::msrHumdrumScotKeyItem (
  int inputLineNumber)
    : msrElement (inputLineNumber)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceKeys) {
    gLogOstream <<
      "Creating Humdrum/Scot key item" <<
      ", line = " << inputLineNumber <<
      endl;
  }
#endif

  fKeyDiatonicPitchKind = k_NoDiatonicPitch;
  fKeyAlterationKind    = k_NoAlteration;
  fKeyOctave            = -1; // actual MusicXML octaves are non-negative
}

msrHumdrumScotKeyItem::~msrHumdrumScotKeyItem ()
{}

bool msrHumdrumScotKeyItem::isEqualTo (
  S_msrHumdrumScotKeyItem
    otherHumdrumScotKeyItem) const
{
  if (! otherHumdrumScotKeyItem) {
    return false;
  }

  return
    fKeyDiatonicPitchKind == otherHumdrumScotKeyItem->fKeyDiatonicPitchKind
      &&
    fKeyAlterationKind == otherHumdrumScotKeyItem->fKeyAlterationKind
      &&
    fKeyOctave == otherHumdrumScotKeyItem->fKeyOctave;
}

void msrHumdrumScotKeyItem::setKeyItemDiatonicPitchKind (
  msrDiatonicPitchKind diatonicPitchKind)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceKeys) {
    gLogOstream <<
      "Setting Humdrum/Scot key item diatonic pitch to '" <<
      msrDiatonicPitchKindAsString (diatonicPitchKind) <<
      "'" <<
      endl;
  }
#endif

  fKeyDiatonicPitchKind = diatonicPitchKind;
}

void msrHumdrumScotKeyItem::setKeyItemAlterationKind (
  msrAlterationKind alterationKind)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceKeys) {
    gLogOstream <<
      "Setting Humdrum/Scot key item alteration to '" <<
      msrAlterationKindAsString (alterationKind) <<
      "'" <<
      endl;
  }
#endif

  fKeyAlterationKind = alterationKind;
}

void msrHumdrumScotKeyItem::setKeyItemOctave (int keyOctave)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceKeys) {
    gLogOstream <<
      "Setting Humdrum/Scot key item octave to '" <<
      keyOctave <<
      "'" <<
      endl;
  }
#endif

  fKeyOctave = keyOctave;
}

void msrHumdrumScotKeyItem::acceptIn (basevisitor* v)
{
  if (gMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrHumdrumScotKeyItem::acceptIn ()" <<
      endl;
  }

  if (visitor<S_msrHumdrumScotKeyItem>*
    p =
      dynamic_cast<visitor<S_msrHumdrumScotKeyItem>*> (v)) {
        S_msrHumdrumScotKeyItem elem = this;

        if (gMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrHumdrumScotKeyItem::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void msrHumdrumScotKeyItem::acceptOut (basevisitor* v)
{
  if (gMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrHumdrumScotKeyItem::acceptOut ()" <<
      endl;
  }

  if (visitor<S_msrHumdrumScotKeyItem>*
    p =
      dynamic_cast<visitor<S_msrHumdrumScotKeyItem>*> (v)) {
        S_msrHumdrumScotKeyItem elem = this;

        if (gMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrHumdrumScotKeyItem::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}

void msrHumdrumScotKeyItem::browseData (basevisitor* v)
{}

string msrHumdrumScotKeyItem::asString () const
{
  stringstream s;

  s <<
    "HumdrumScotKeyItem" <<
    ", KeyDiatonicPitch" << ": " <<
    msrDiatonicPitchKindAsString (fKeyDiatonicPitchKind) <<
    ", KeyAlteration" << ": " <<
    msrAlterationKindAsString (fKeyAlterationKind) <<
    ", KeyOctave" << ": " << fKeyOctave <<
    ", line " << fInputLineNumber;

  return s.str ();
}

void msrHumdrumScotKeyItem::print (ostream& os) const
{
  os <<
    asString () <<
    endl;
}

ostream& operator<< (ostream& os, const S_msrHumdrumScotKeyItem& elt)
{
  elt->print (os);
  return os;
}

//______________________________________________________________________________
S_msrKey msrKey::createTraditional (
  int                      inputLineNumber,
  msrQuarterTonesPitchKind keyTonicQuarterTonesPitchKind,
  msrKeyModeKind           keyModeKind,
  int                      keyCancel)
{
  msrKey* o =
    new msrKey (
      inputLineNumber,
      keyTonicQuarterTonesPitchKind, keyModeKind,
      keyCancel);
  assert (o!=0);

  return o;
}

S_msrKey msrKey::createHumdrumScot (
  int inputLineNumber)
{
  msrKey* o =
    new msrKey (
      inputLineNumber);
  assert (o!=0);

  return o;
}

msrKey::msrKey ( // for traditional keys
  int                      inputLineNumber,
  msrQuarterTonesPitchKind keyTonicQuarterTonesPitchKind,
  msrKeyModeKind           keyModeKind,
  int                      keyCancel)
    : msrMeasureElement (inputLineNumber)
{
  // this is a traditional key
  fKeyKind = kTraditionalKind;

  fKeyModeKind = keyModeKind;

  /* caution:
    <key>
      <fifths>0</fifths>
      <mode>minor</mode>
    </key>
  is A minor actually, not C minor!
  */

  fKeyTonicQuarterTonesPitchKind = keyTonicQuarterTonesPitchKind;

  switch (fKeyModeKind) {
    case msrKey::kMajorMode:
      break;
    case msrKey::kMinorMode:
      fKeyTonicQuarterTonesPitchKind =
        noteAtIntervalFromQuarterTonesPitch (
          inputLineNumber,
          kMajorSixth, // a minor third below actually
          fKeyTonicQuarterTonesPitchKind);
      break;
    case msrKey::kIonianMode:
      break;
    case msrKey::kDorianMode:
      break;
    case msrKey::kPhrygianMode:
      break;
    case msrKey::kLydianMode:
      break;
    case msrKey::kMixolydianMode:
      break;
    case msrKey::kAeolianMode:
      break;
    case msrKey::kLocrianMode:
      break;
  } // switch

  fKeyCancel = keyCancel;

  // initialization in all cases
  fKeyItemsOctavesAreSpecified = false;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceKeys) {
    gLogOstream <<
      "Creating traditional key '" <<
      this->asString () <<
      "'" <<
      endl;
    }
#endif
}

msrKey::msrKey ( // for Humdrum/Scot keys
  int                  inputLineNumber)
    : msrMeasureElement (inputLineNumber)
{
  // this is a Humdrum/Scot key
  fKeyKind = kHumdrumScotKind;

  // initialization in all cases
  fKeyItemsOctavesAreSpecified = false;

#ifdef TRACE_OAH
  if (gTraceOah->fTraceKeys) {
    gLogOstream <<
      "Creating Humdrum/Scot key '" <<
      this->asString () <<
      "'" <<
      endl;
    }
#endif
}

msrKey::~msrKey ()
{}

bool msrKey::isEqualTo (S_msrKey otherKey) const
{
  if (! otherKey) {
    return false;
  }

  if (
    ! (
        fKeyKind == otherKey->fKeyKind
          &&
        fKeyTonicQuarterTonesPitchKind
          ==
        otherKey->fKeyTonicQuarterTonesPitchKind
          &&
        fKeyModeKind == otherKey->fKeyModeKind
          &&
        fKeyCancel == otherKey->fKeyCancel
      )
    ) {
    return false;
  }

  switch (fKeyKind) {
    case msrKey::kTraditionalKind:
      break;
    case msrKey::kHumdrumScotKind:
      {
        if (
          ! (
              fKeyItemsOctavesAreSpecified
                ==
              otherKey->fKeyItemsOctavesAreSpecified
                  &&
              fHumdrumScotKeyItemsVector.size ()
                ==
              otherKey->fHumdrumScotKeyItemsVector.size ()
            )
          ) {
          return false;
        }

        for (unsigned int i = 0; i < fHumdrumScotKeyItemsVector.size (); i++) {
          if (
            ! (
              fHumdrumScotKeyItemsVector [i]->isEqualTo (
                otherKey->fHumdrumScotKeyItemsVector [i])
              )
            ) {
            return false;
          }
        } // for
      }
      break;
  } // switch

  return true;
}

void msrKey::appendHumdrumScotKeyItem (
  S_msrHumdrumScotKeyItem item)
{
#ifdef TRACE_OAH
  if (gTraceOah->fTraceKeys) {
    gLogOstream <<
      "Append item '" <<
      item->asString () <<
      "' to key '" <<
      "'" <<
      endl;
    }
#endif

  // have key items octaves been specified?
  if (item->getKeyItemOctave () >= 0) {
    fKeyItemsOctavesAreSpecified = true;
  }

  // append the item to the vector
  fHumdrumScotKeyItemsVector.insert (
    fHumdrumScotKeyItemsVector.end (), item);
}

void msrKey::acceptIn (basevisitor* v)
{
  if (gMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrKey::acceptIn ()" <<
      endl;
  }

  if (visitor<S_msrKey>*
    p =
      dynamic_cast<visitor<S_msrKey>*> (v)) {
        S_msrKey elem = this;

        if (gMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrKey::visitStart ()" <<
            endl;
        }
        p->visitStart (elem);
  }
}

void msrKey::acceptOut (basevisitor* v)
{
  if (gMsrOah->fTraceMsrVisitors) {
    gLogOstream <<
      "% ==> msrKey::acceptOut ()" <<
      endl;
  }

  if (visitor<S_msrKey>*
    p =
      dynamic_cast<visitor<S_msrKey>*> (v)) {
        S_msrKey elem = this;

        if (gMsrOah->fTraceMsrVisitors) {
          gLogOstream <<
            "% ==> Launching msrKey::visitEnd ()" <<
            endl;
        }
        p->visitEnd (elem);
  }
}

void msrKey::browseData (basevisitor* v)
{}

string msrKey::keyKindAsString (
  msrKeyKind keyKind)
{
  string result;

  switch (keyKind) {
    case msrKey::kTraditionalKind:
      result = "traditional";
      break;
    case msrKey::kHumdrumScotKind:
      result = "Humdrum/Scot";
      break;
  } // switch

  return result;
}

string msrKey::keyModeKindAsString (
  msrKeyModeKind keyModeKind)
{
  string result;

  switch (keyModeKind) {
    case msrKey::kMajorMode:
      result = "major";
      break;
    case msrKey::kMinorMode:
      result = "minor";
      break;
    case msrKey::kIonianMode:
      result = "ionian";
      break;
    case msrKey::kDorianMode:
      result = "dorian";
      break;
    case msrKey::kPhrygianMode:
      result = "phrygian";
      break;
    case msrKey::kLydianMode:
      result = "lydian";
      break;
    case msrKey::kMixolydianMode:
      result = "mixolydian";
      break;
    case msrKey::kAeolianMode:
      result = "aeolian";
      break;
    case msrKey::kLocrianMode:
      result = "locrian";
      break;
  } // switch

  return result;
}

string msrKey::asString () const
{
  stringstream s;

  s <<
    "[Key " <<
    keyKindAsString (fKeyKind) <<
    ", ";

  switch (fKeyKind) {
    case msrKey::kTraditionalKind:
      s <<
        msrQuarterTonesPitchKindAsString (
          gMsrOah->fMsrQuarterTonesPitchesLanguageKind,
          fKeyTonicQuarterTonesPitchKind) <<
        " " <<
        keyModeKindAsString (fKeyModeKind);
      break;

    case msrKey::kHumdrumScotKind:
      s <<
        fHumdrumScotKeyItemsVector.size () << "items" <<
        ", keyItemsOctavesAreSpecified: " <<
        booleanAsString (
          fKeyItemsOctavesAreSpecified);
      break;
  } // switch

  s <<
    ", line " << fInputLineNumber <<
    "]";

  return s.str ();
}

void msrKey::print (ostream& os) const
{
  os <<
    "Key" <<
    ", " << keyKindAsString (fKeyKind) <<
    ",";

  switch (fKeyKind) {
    case msrKey::kTraditionalKind:
      os <<
        " " <<
        msrQuarterTonesPitchKindAsString (
          gMsrOah->fMsrQuarterTonesPitchesLanguageKind,
          fKeyTonicQuarterTonesPitchKind) <<
        " " <<
        keyModeKindAsString (fKeyModeKind) <<
        ", line " << fInputLineNumber <<
        endl;
      break;

    case msrKey::kHumdrumScotKind:
      os <<
        ", keyItemsOctavesAreSpecified: " <<
        booleanAsString (
          fKeyItemsOctavesAreSpecified) <<
        ", " <<
        fHumdrumScotKeyItemsVector.size () <<
        " items" <<
        ", line " << fInputLineNumber <<
        endl;

      if (fHumdrumScotKeyItemsVector.size ()) {
        os << endl;

        gIndenter++;

        vector<S_msrHumdrumScotKeyItem>::const_iterator
          iBegin = fHumdrumScotKeyItemsVector.begin (),
          iEnd   = fHumdrumScotKeyItemsVector.end (),
          i      = iBegin;

        for ( ; ; ) {
          os << (*i);
          if (++i == iEnd) break;
    // JMI     os << endl;
        } // for

        gIndenter--;
      }

      else
        {
          os <<
            " none" <<
            endl;
        }
      break;
  } // switch
}

ostream& operator<< (ostream& os, const S_msrKey& elt)
{
  elt->print (os);
  return os;
}


}
