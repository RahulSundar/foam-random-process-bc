/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     3.2
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "randomProcessFixedValueFvPatchField.H"
#include "mathematicalConstants.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Type>
scalar randomProcessFixedValueFvPatchField<Type>::randomPhase()
{
    return randGen_.scalar01() * 2*mathematicalConstant::pi;
}

template<class Type>
scalar randomProcessFixedValueFvPatchField<Type>::generate()
{
    scalar x = 0.0;
    scalar dw = circFreq_[1]-circFreq_[0];

    forAll(circFreq_, i)
    {
        //x += sqrt(2*psd_[i]) * cos(circFreq_[i]*this->db().time().value() + randomPhase());
        x += sqrt(2*dw*psd_[i]) * cos(circFreq_[i]*this->db().time().value() + randomPhase());
    }

    return x;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
randomProcessFixedValueFvPatchField<Type>::randomProcessFixedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(p, iF),
    gustDir_(p.size()),
    scaleFac_(1.0),
    circFreq_(scalarList(2, 0.0)),
    psd_(scalarList(2, 0.0)),
    randGen_(label(0)),
    curTimeIndex_(-1)
{}


template<class Type>
randomProcessFixedValueFvPatchField<Type>::randomProcessFixedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<Type>(p, iF),
    gustDir_("gustDir", dict, p.size()),
    scaleFac_(readScalar(dict.lookup("scaleFac"))),
    circFreq_(dict.lookup("circFreq")),
    psd_(dict.lookup("psd")),
    randGen_(label(0)),
    curTimeIndex_(-1)
{
    if (dict.found("value"))
    {
        fixedValueFvPatchField<Type>::operator==
        (
            Field<Type>("value", dict, p.size())
        );
    }
    else
    {
        fixedValueFvPatchField<Type>::operator==(scaleFac_*gustDir_*generate());
    }
}


template<class Type>
randomProcessFixedValueFvPatchField<Type>::randomProcessFixedValueFvPatchField
(
    const randomProcessFixedValueFvPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<Type>(ptf, p, iF, mapper),
    gustDir_(ptf.gustDir_, mapper),
    scaleFac_(ptf.scaleFac_),
    circFreq_(ptf.circFreq_),
    psd_(ptf.psd_),
    randGen_(ptf.randGen_),
    curTimeIndex_(-1)
{}


template<class Type>
randomProcessFixedValueFvPatchField<Type>::randomProcessFixedValueFvPatchField
(
    const randomProcessFixedValueFvPatchField<Type>& ptf
)
:
    fixedValueFvPatchField<Type>(ptf),
    gustDir_(ptf.gustDir_),
    scaleFac_(ptf.scaleFac_),
    circFreq_(ptf.circFreq_),
    psd_(ptf.psd_),
    randGen_(ptf.randGen_),
    curTimeIndex_(-1)
{}


template<class Type>
randomProcessFixedValueFvPatchField<Type>::randomProcessFixedValueFvPatchField
(
    const randomProcessFixedValueFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(ptf, iF),
    gustDir_(ptf.gustDir_),
    scaleFac_(ptf.scaleFac_),
    circFreq_(ptf.circFreq_),
    psd_(ptf.psd_),
    randGen_(ptf.randGen_),
    curTimeIndex_(-1)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void randomProcessFixedValueFvPatchField<Type>::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedValueFvPatchField<Type>::autoMap(m);
    gustDir_.autoMap(m);
}


template<class Type>
void randomProcessFixedValueFvPatchField<Type>::rmap
(
    const fvPatchField<Type>& ptf,
    const labelList& addr
)
{
    fixedValueFvPatchField<Type>::rmap(ptf, addr);

    const randomProcessFixedValueFvPatchField<Type>& tiptf =
        refCast<const randomProcessFixedValueFvPatchField<Type> >(ptf);

    gustDir_.rmap(tiptf.gustDir_, addr);
}


template<class Type>
void randomProcessFixedValueFvPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    if (curTimeIndex_ != this->db().time().timeIndex())
    {
        Field<Type>& patchField = *this;
        patchField = scaleFac_*gustDir_*generate();

        //const vectorField& Cf = patch().Cf();

        //forAll(patchField, facei)
        //{
        //    patchField[facei] = generate() * gustDir_;
        //}

        //const vectorField Ui = generate() * ((Cf-Cf) + gustDir_);
        //vectorField::operator=(Ui);
        //patchField = Ui;

        curTimeIndex_ = this->db().time().timeIndex();
    }

    fixedValueFvPatchField<Type>::updateCoeffs();
}


template<class Type>
void randomProcessFixedValueFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);
    gustDir_.writeEntry("gustDir", os);
    os.writeKeyword("scaleFac")
        << scaleFac_ << token::END_STATEMENT << nl;
    // os.writeKeyword("frequency")
    //     << frequency_ << token::END_STATEMENT << nl;
    this->writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //