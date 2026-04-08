#include "CharacterArchetypeManager.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

FCharacterVariationData UCharacterArchetypeManager::GenerateRandomVariation(ECharacterArchetype Archetype)
{
    FCharacterVariationData NewVariation;
    NewVariation.Archetype = Archetype;
    
    // Gerar nome único baseado no arquétipo
    NewVariation.CharacterName = GenerateArchetypeName(Archetype);
    
    // Selecionar tipo corporal baseado no arquétipo
    NewVariation.BodyType = SelectBodyTypeForArchetype(Archetype);
    
    // Gerar tom de pele (variação natural)
    int32 SkinToneIndex = FMath::RandRange(0, static_cast<int32>(ECharacterSkinTone::VeryDark));
    NewVariation.SkinTone = static_cast<ECharacterSkinTone>(SkinToneIndex);
    
    // Gerar características faciais procedurais
    NewVariation.FaceWidth = GenerateBlendShapeValue(Archetype, TEXT("FaceWidth"));
    NewVariation.EyeSize = GenerateBlendShapeValue(Archetype, TEXT("EyeSize"));
    NewVariation.NoseSize = GenerateBlendShapeValue(Archetype, TEXT("NoseSize"));
    NewVariation.LipThickness = GenerateBlendShapeValue(Archetype, TEXT("LipThickness"));
    NewVariation.CheekboneHeight = GenerateBlendShapeValue(Archetype, TEXT("CheekboneHeight"));
    NewVariation.JawWidth = GenerateBlendShapeValue(Archetype, TEXT("JawWidth"));
    
    // Selecionar cabelo
    NewVariation.HairAssetPath = SelectHairAsset(Archetype, NewVariation.BodyType);
    NewVariation.HairColor = GenerateHairColor(Archetype);
    
    // Selecionar roupa
    NewVariation.ClothingType = SelectClothing(Archetype);
    
    // Selecionar acessórios
    NewVariation.AccessoryPaths = SelectAccessories(Archetype);
    
    // Gerar marcas de vida (cicatrizes, tatuagens)
    NewVariation.LifeMarkTextures = GenerateLifeMarks(Archetype);
    
    // Definir animações idle
    NewVariation.IdleAnimationSet = SelectIdleAnimationSet(Archetype);
    
    // Gerar variação de postura
    NewVariation.PostureVariation = GeneratePostureVariation(Archetype);
    
    return NewVariation;
}

FCharacterVariationData UCharacterArchetypeManager::GetSpecificVariation(ECharacterArchetype Archetype, int32 VariationIndex)
{
    if (ArchetypeVariations.Contains(Archetype))
    {
        const TArray<FCharacterVariationData>& Variations = ArchetypeVariations[Archetype];
        if (Variations.IsValidIndex(VariationIndex))
        {
            return Variations[VariationIndex];
        }
    }
    
    // Se não encontrar, gerar nova variação
    return GenerateRandomVariation(Archetype);
}

void UCharacterArchetypeManager::PopulateArchetypeDatabase()
{
    // Limpar database existente
    ArchetypeVariations.Empty();
    
    // Gerar variações para cada arquétipo
    for (int32 ArchetypeIndex = 0; ArchetypeIndex < static_cast<int32>(ECharacterArchetype::MAX); ++ArchetypeIndex)
    {
        ECharacterArchetype CurrentArchetype = static_cast<ECharacterArchetype>(ArchetypeIndex);
        TArray<FCharacterVariationData> Variations;
        
        for (int32 i = 0; i < MaxVariationsPerArchetype; ++i)
        {
            Variations.Add(GenerateRandomVariation(CurrentArchetype));
        }
        
        ArchetypeVariations.Add(CurrentArchetype, Variations);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Character Database populated with %d archetypes"), ArchetypeVariations.Num());
}

float UCharacterArchetypeManager::GenerateBlendShapeValue(ECharacterArchetype Archetype, const FString& BlendShapeName)
{
    // Valores base por arquétipo para criar personalidade visual
    float BaseValue = 0.5f;
    float Variance = 0.3f;
    
    switch (Archetype)
    {
        case ECharacterArchetype::TribalElder:
            if (BlendShapeName == TEXT("CheekboneHeight")) BaseValue = 0.7f;
            if (BlendShapeName == TEXT("JawWidth")) BaseValue = 0.6f;
            Variance = 0.2f; // Menos variação para anciãos
            break;
            
        case ECharacterArchetype::TribalHunter:
            if (BlendShapeName == TEXT("JawWidth")) BaseValue = 0.7f;
            if (BlendShapeName == TEXT("CheekboneHeight")) BaseValue = 0.6f;
            Variance = 0.25f;
            break;
            
        case ECharacterArchetype::TribalShaman:
            if (BlendShapeName == TEXT("EyeSize")) BaseValue = 0.6f;
            if (BlendShapeName == TEXT("FaceWidth")) BaseValue = 0.4f;
            Variance = 0.35f; // Mais variação para xamãs (mais únicos)
            break;
            
        case ECharacterArchetype::CannibalRaider:
            if (BlendShapeName == TEXT("JawWidth")) BaseValue = 0.8f;
            if (BlendShapeName == TEXT("EyeSize")) BaseValue = 0.3f;
            Variance = 0.4f; // Muito variados, mais ameaçadores
            break;
            
        case ECharacterArchetype::TribalChild:
            if (BlendShapeName == TEXT("EyeSize")) BaseValue = 0.8f;
            if (BlendShapeName == TEXT("FaceWidth")) BaseValue = 0.6f;
            if (BlendShapeName == TEXT("JawWidth")) BaseValue = 0.3f;
            Variance = 0.2f;
            break;
    }
    
    // Aplicar variação aleatória
    float RandomOffset = FMath::RandRange(-Variance, Variance);
    return FMath::Clamp(BaseValue + RandomOffset, 0.0f, 1.0f);
}

ECharacterBodyType UCharacterArchetypeManager::SelectBodyTypeForArchetype(ECharacterArchetype Archetype)
{
    switch (Archetype)
    {
        case ECharacterArchetype::TribalHunter:
        case ECharacterArchetype::CannibalRaider:
            // Caçadores e invasores tendem a ser mais musculosos
            return FMath::RandBool() ? ECharacterBodyType::Masculine_Average : ECharacterBodyType::Masculine_Heavy;
            
        case ECharacterArchetype::TribalElder:
            return FMath::RandBool() ? ECharacterBodyType::Elder_Frail : ECharacterBodyType::Masculine_Lean;
            
        case ECharacterArchetype::TribalChild:
            return FMath::RandBool() ? ECharacterBodyType::Child_Small : ECharacterBodyType::Child_Medium;
            
        case ECharacterArchetype::TribalShaman:
            // Xamãs podem ser qualquer tipo - mais variação
            return static_cast<ECharacterBodyType>(FMath::RandRange(0, 5));
            
        default:
            // Distribuição natural para outros arquétipos
            int32 RandomType = FMath::RandRange(0, 5);
            return static_cast<ECharacterBodyType>(RandomType);
    }
}

FString UCharacterArchetypeManager::SelectHairAsset(ECharacterArchetype Archetype, ECharacterBodyType BodyType)
{
    // Paths para assets de cabelo MetaHuman
    TArray<FString> MaleHairAssets = {
        TEXT("/Game/MetaHumans/Common/Hair/Male/Short_Messy"),
        TEXT("/Game/MetaHumans/Common/Hair/Male/Long_Braided"),
        TEXT("/Game/MetaHumans/Common/Hair/Male/Medium_Wild"),
        TEXT("/Game/MetaHumans/Common/Hair/Male/Bald_Weathered")
    };
    
    TArray<FString> FemaleHairAssets = {
        TEXT("/Game/MetaHumans/Common/Hair/Female/Long_Loose"),
        TEXT("/Game/MetaHumans/Common/Hair/Female/Medium_Braided"),
        TEXT("/Game/MetaHumans/Common/Hair/Female/Short_Practical"),
        TEXT("/Game/MetaHumans/Common/Hair/Female/Decorated_Tribal")
    };
    
    TArray<FString> ChildHairAssets = {
        TEXT("/Game/MetaHumans/Common/Hair/Child/Messy_Short"),
        TEXT("/Game/MetaHumans/Common/Hair/Child/Braided_Simple")
    };
    
    // Selecionar baseado no tipo corporal
    if (BodyType == ECharacterBodyType::Child_Small || BodyType == ECharacterBodyType::Child_Medium)
    {
        return ChildHairAssets[FMath::RandRange(0, ChildHairAssets.Num() - 1)];
    }
    else if (BodyType == ECharacterBodyType::Feminine_Lean || 
             BodyType == ECharacterBodyType::Feminine_Average || 
             BodyType == ECharacterBodyType::Feminine_Heavy)
    {
        return FemaleHairAssets[FMath::RandRange(0, FemaleHairAssets.Num() - 1)];
    }
    else
    {
        return MaleHairAssets[FMath::RandRange(0, MaleHairAssets.Num() - 1)];
    }
}

FLinearColor UCharacterArchetypeManager::GenerateHairColor(ECharacterArchetype Archetype)
{
    // Cores de cabelo naturais para período pré-histórico
    TArray<FLinearColor> NaturalHairColors = {
        FLinearColor(0.1f, 0.05f, 0.02f, 1.0f),  // Preto
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),   // Castanho escuro
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f),    // Castanho
        FLinearColor(0.4f, 0.3f, 0.15f, 1.0f),   // Castanho claro
        FLinearColor(0.6f, 0.6f, 0.6f, 1.0f),    // Grisalho (anciãos)
        FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)     // Branco (anciãos)
    };
    
    if (Archetype == ECharacterArchetype::TribalElder)
    {
        // Anciãos têm cabelo grisalho/branco
        return NaturalHairColors[FMath::RandRange(4, 5)];
    }
    
    return NaturalHairColors[FMath::RandRange(0, 3)];
}

ECharacterClothing UCharacterArchetypeManager::SelectClothing(ECharacterArchetype Archetype)
{
    switch (Archetype)
    {
        case ECharacterArchetype::TribalShaman:
            return ECharacterClothing::ShamanRobes;
            
        case ECharacterArchetype::TribalHunter:
        case ECharacterArchetype::CannibalRaider:
            return ECharacterClothing::HunterGear;
            
        case ECharacterArchetype::TribalGatherer:
            return ECharacterClothing::GathererPouch;
            
        case ECharacterArchetype::LoneWanderer:
        case ECharacterArchetype::CaveHermit:
            return ECharacterClothing::Tattered;
            
        default:
            // Distribuição aleatória de roupas básicas
            TArray<ECharacterClothing> BasicClothing = {
                ECharacterClothing::AnimalHides,
                ECharacterClothing::WovenFibers,
                ECharacterClothing::BarkCloth,
                ECharacterClothing::LeatherWraps
            };
            return BasicClothing[FMath::RandRange(0, BasicClothing.Num() - 1)];
    }
}

TArray<FString> UCharacterArchetypeManager::SelectAccessories(ECharacterArchetype Archetype)
{
    TArray<FString> Accessories;
    
    switch (Archetype)
    {
        case ECharacterArchetype::TribalShaman:
            Accessories.Add(TEXT("/Game/Characters/Accessories/Bone_Necklace"));
            Accessories.Add(TEXT("/Game/Characters/Accessories/Feather_Headpiece"));
            Accessories.Add(TEXT("/Game/Characters/Accessories/Spirit_Pouch"));
            break;
            
        case ECharacterArchetype::TribalHunter:
            Accessories.Add(TEXT("/Game/Characters/Accessories/Hunting_Spear"));
            Accessories.Add(TEXT("/Game/Characters/Accessories/Stone_Knife"));
            if (FMath::RandBool()) Accessories.Add(TEXT("/Game/Characters/Accessories/Trophy_Teeth"));
            break;
            
        case ECharacterArchetype::TribalElder:
            Accessories.Add(TEXT("/Game/Characters/Accessories/Walking_Staff"));
            if (FMath::RandBool()) Accessories.Add(TEXT("/Game/Characters/Accessories/Wisdom_Beads"));
            break;
            
        case ECharacterArchetype::CannibalRaider:
            Accessories.Add(TEXT("/Game/Characters/Accessories/Bone_Weapon"));
            Accessories.Add(TEXT("/Game/Characters/Accessories/War_Paint"));
            if (FMath::RandBool()) Accessories.Add(TEXT("/Game/Characters/Accessories/Skull_Trophy"));
            break;
    }
    
    return Accessories;
}

TArray<FString> UCharacterArchetypeManager::GenerateLifeMarks(ECharacterArchetype Archetype)
{
    TArray<FString> LifeMarks;
    
    // Probabilidade de ter marcas baseada no arquétipo
    float ScarProbability = 0.3f;
    float TattooProbability = 0.2f;
    
    switch (Archetype)
    {
        case ECharacterArchetype::TribalHunter:
        case ECharacterArchetype::CannibalRaider:
            ScarProbability = 0.7f;
            TattooProbability = 0.5f;
            break;
            
        case ECharacterArchetype::TribalShaman:
            ScarProbability = 0.2f;
            TattooProbability = 0.8f; // Xamãs têm mais tatuagens rituais
            break;
            
        case ECharacterArchetype::TribalElder:
            ScarProbability = 0.6f; // Vida longa = mais cicatrizes
            TattooProbability = 0.4f;
            break;
    }
    
    if (FMath::RandRange(0.0f, 1.0f) < ScarProbability)
    {
        TArray<FString> ScarTextures = {
            TEXT("/Game/Characters/LifeMarks/Scars/Face_Scar_01"),
            TEXT("/Game/Characters/LifeMarks/Scars/Arm_Scar_01"),
            TEXT("/Game/Characters/LifeMarks/Scars/Chest_Scar_01")
        };
        LifeMarks.Add(ScarTextures[FMath::RandRange(0, ScarTextures.Num() - 1)]);
    }
    
    if (FMath::RandRange(0.0f, 1.0f) < TattooProbability)
    {
        TArray<FString> TattooTextures = {
            TEXT("/Game/Characters/LifeMarks/Tattoos/Tribal_Face_01"),
            TEXT("/Game/Characters/LifeMarks/Tattoos/Ritual_Arm_01"),
            TEXT("/Game/Characters/LifeMarks/Tattoos/Spirit_Chest_01")
        };
        LifeMarks.Add(TattooTextures[FMath::RandRange(0, TattooTextures.Num() - 1)]);
    }
    
    return LifeMarks;
}

FString UCharacterArchetypeManager::SelectIdleAnimationSet(ECharacterArchetype Archetype)
{
    switch (Archetype)
    {
        case ECharacterArchetype::TribalElder:
            return TEXT("/Game/Characters/Animations/Elder_Idle_Set");
            
        case ECharacterArchetype::TribalHunter:
            return TEXT("/Game/Characters/Animations/Hunter_Idle_Set");
            
        case ECharacterArchetype::TribalShaman:
            return TEXT("/Game/Characters/Animations/Shaman_Idle_Set");
            
        case ECharacterArchetype::CannibalRaider:
            return TEXT("/Game/Characters/Animations/Aggressive_Idle_Set");
            
        case ECharacterArchetype::TribalChild:
            return TEXT("/Game/Characters/Animations/Child_Idle_Set");
            
        default:
            return TEXT("/Game/Characters/Animations/Default_Idle_Set");
    }
}

float UCharacterArchetypeManager::GeneratePostureVariation(ECharacterArchetype Archetype)
{
    switch (Archetype)
    {
        case ECharacterArchetype::TribalElder:
            return FMath::RandRange(-0.8f, -0.3f); // Mais curvado
            
        case ECharacterArchetype::TribalHunter:
        case ECharacterArchetype::CannibalRaider:
            return FMath::RandRange(0.3f, 0.8f); // Mais ereto, alerta
            
        case ECharacterArchetype::CaveHermit:
        case ECharacterArchetype::MadSurvivor:
            return FMath::RandRange(-0.6f, -0.2f); // Postura defensiva
            
        default:
            return FMath::RandRange(-0.3f, 0.3f); // Variação normal
    }
}

FString UCharacterArchetypeManager::GenerateArchetypeName(ECharacterArchetype Archetype)
{
    TArray<FString> Names;
    
    switch (Archetype)
    {
        case ECharacterArchetype::TribalElder:
            Names = {TEXT("Kael"), TEXT("Thora"), TEXT("Bran"), TEXT("Sira"), TEXT("Ulf")};
            break;
        case ECharacterArchetype::TribalHunter:
            Names = {TEXT("Gor"), TEXT("Vera"), TEXT("Dax"), TEXT("Nira"), TEXT("Rex")};
            break;
        case ECharacterArchetype::TribalShaman:
            Names = {TEXT("Zara"), TEXT("Mor"), TEXT("Luna"), TEXT("Keth"), TEXT("Yara")};
            break;
        default:
            Names = {TEXT("Ash"), TEXT("Rok"), TEXT("Lin"), TEXT("Tor"), TEXT("Nia")};
    }
    
    FString BaseName = Names[FMath::RandRange(0, Names.Num() - 1)];
    int32 Suffix = FMath::RandRange(1, 999);
    
    return FString::Printf(TEXT("%s_%d"), *BaseName, Suffix);
}