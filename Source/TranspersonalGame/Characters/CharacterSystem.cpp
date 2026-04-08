#include "CharacterSystem.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

UCharacterGenerationSystem::UCharacterGenerationSystem()
{
    // Inicializar sistema
}

FCharacterDefinition UCharacterGenerationSystem::GenerateCharacter(ECharacterArchetype Archetype, int32 Seed)
{
    // Se seed não foi fornecido, gerar um aleatório
    if (Seed == -1)
    {
        Seed = FMath::Rand();
    }

    FCharacterDefinition NewCharacter;
    NewCharacter.Archetype = Archetype;
    NewCharacter.GeneticSeed = Seed;

    // Usar seed local para geração consistente
    int32 LocalSeed = Seed;

    // Gerar características básicas
    NewCharacter.bIsMale = GenerateRandomFloat(LocalSeed) > 0.5f;
    
    // Idade baseada no arquétipo
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            NewCharacter.Age = FMath::RandRange(30, 45);
            break;
        case ECharacterArchetype::TribalElder:
            NewCharacter.Age = FMath::RandRange(50, 70);
            break;
        case ECharacterArchetype::TribalChild:
            NewCharacter.Age = FMath::RandRange(8, 16);
            break;
        case ECharacterArchetype::TribalWarrior:
            NewCharacter.Age = FMath::RandRange(20, 40);
            break;
        case ECharacterArchetype::TribalShaman:
            NewCharacter.Age = FMath::RandRange(35, 60);
            break;
        default:
            NewCharacter.Age = FMath::RandRange(18, 50);
            break;
    }

    // Gerar características genéticas
    NewCharacter.GeneticTraits = GenerateGeneticVariation(FCharacterGeneticTraits(), 0.3f, LocalSeed);

    // Ajustar características baseadas na idade e arquétipo
    if (Archetype == ECharacterArchetype::TribalElder)
    {
        NewCharacter.GeneticTraits.AgeWear = GenerateRandomFloat(LocalSeed, 0.6f, 0.9f);
        NewCharacter.GeneticTraits.WeatherExposure = GenerateRandomFloat(LocalSeed, 0.7f, 1.0f);
    }
    else if (Archetype == ECharacterArchetype::TribalWarrior)
    {
        NewCharacter.GeneticTraits.MuscleDefinition = GenerateRandomFloat(LocalSeed, 0.7f, 1.0f);
        NewCharacter.GeneticTraits.BattleScars = GenerateRandomFloat(LocalSeed, 0.3f, 0.8f);
    }
    else if (Archetype == ECharacterArchetype::Protagonist)
    {
        // Paleontologista moderno - menos desgaste, mais definição acadêmica
        NewCharacter.GeneticTraits.AgeWear = GenerateRandomFloat(LocalSeed, 0.1f, 0.3f);
        NewCharacter.GeneticTraits.MuscleDefinition = GenerateRandomFloat(LocalSeed, 0.3f, 0.6f);
        NewCharacter.GeneticTraits.WeatherExposure = 0.0f; // Recém chegado
    }

    // Gerar aparência cultural
    NewCharacter.CulturalAppearance.SkinTone = GenerateNaturalSkinTone(LocalSeed);
    NewCharacter.CulturalAppearance.EyeColor = GenerateNaturalEyeColor(LocalSeed);
    NewCharacter.CulturalAppearance.HairColor = GenerateNaturalHairColor(LocalSeed);
    NewCharacter.CulturalAppearance.HairStyle = SelectRandomHairStyle(LocalSeed, NewCharacter.bIsMale);
    
    // Marcas tribais baseadas no arquétipo
    if (Archetype != ECharacterArchetype::Protagonist && Archetype != ECharacterArchetype::TimeTraveler)
    {
        NewCharacter.CulturalAppearance.BodyMarkings = GenerateTribalMarkings(LocalSeed, Archetype);
    }

    // Gerar vestuário
    GenerateClothing(NewCharacter, LocalSeed);

    // Gerar nome baseado no arquétipo
    NewCharacter.CharacterName = GenerateCharacterName(Archetype, NewCharacter.bIsMale, LocalSeed);

    // Verificar unicidade
    int32 UniqueAttempts = 0;
    while (!IsCharacterUnique(NewCharacter) && UniqueAttempts < 10)
    {
        LocalSeed = FMath::Rand();
        NewCharacter = GenerateCharacter(Archetype, LocalSeed);
        UniqueAttempts++;
    }

    // Adicionar à lista de personagens gerados
    GeneratedCharacters.Add(NewCharacter);

    return NewCharacter;
}

FCharacterGeneticTraits UCharacterGenerationSystem::GenerateGeneticVariation(const FCharacterGeneticTraits& BaseTraits, float VariationStrength, int32 Seed)
{
    FCharacterGeneticTraits VariedTraits = BaseTraits;
    int32 LocalSeed = Seed;

    // Aplicar variação a cada característica
    VariedTraits.FaceWidth = FMath::Clamp(BaseTraits.FaceWidth + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);
    VariedTraits.EyeSize = FMath::Clamp(BaseTraits.EyeSize + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);
    VariedTraits.NoseSize = FMath::Clamp(BaseTraits.NoseSize + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);
    VariedTraits.MouthSize = FMath::Clamp(BaseTraits.MouthSize + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);
    VariedTraits.CheekboneHeight = FMath::Clamp(BaseTraits.CheekboneHeight + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);
    VariedTraits.JawWidth = FMath::Clamp(BaseTraits.JawWidth + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);
    VariedTraits.BodyHeight = FMath::Clamp(BaseTraits.BodyHeight + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);
    VariedTraits.BodyWeight = FMath::Clamp(BaseTraits.BodyWeight + GenerateRandomFloat(LocalSeed, -VariationStrength, VariationStrength), 0.0f, 1.0f);

    return VariedTraits;
}

bool UCharacterGenerationSystem::IsCharacterUnique(const FCharacterDefinition& Character, float SimilarityThreshold)
{
    for (const FCharacterDefinition& ExistingChar : GeneratedCharacters)
    {
        // Calcular similaridade facial
        float FacialSimilarity = 0.0f;
        FacialSimilarity += FMath::Abs(Character.GeneticTraits.FaceWidth - ExistingChar.GeneticTraits.FaceWidth);
        FacialSimilarity += FMath::Abs(Character.GeneticTraits.EyeSize - ExistingChar.GeneticTraits.EyeSize);
        FacialSimilarity += FMath::Abs(Character.GeneticTraits.NoseSize - ExistingChar.GeneticTraits.NoseSize);
        FacialSimilarity += FMath::Abs(Character.GeneticTraits.MouthSize - ExistingChar.GeneticTraits.MouthSize);
        FacialSimilarity += FMath::Abs(Character.GeneticTraits.CheekboneHeight - ExistingChar.GeneticTraits.CheekboneHeight);
        FacialSimilarity += FMath::Abs(Character.GeneticTraits.JawWidth - ExistingChar.GeneticTraits.JawWidth);
        
        FacialSimilarity /= 6.0f; // Média das diferenças
        
        // Se a similaridade for muito alta, não é único
        if (FacialSimilarity < (1.0f - SimilarityThreshold))
        {
            return false;
        }
    }
    
    return true;
}

float UCharacterGenerationSystem::GenerateRandomFloat(int32& Seed, float Min, float Max)
{
    // Implementação simples de gerador pseudo-aleatório baseado em seed
    Seed = (Seed * 1103515245 + 12345) & 0x7fffffff;
    float NormalizedValue = (float)Seed / (float)0x7fffffff;
    return Min + (Max - Min) * NormalizedValue;
}

FLinearColor UCharacterGenerationSystem::GenerateNaturalSkinTone(int32& Seed)
{
    // Tons de pele naturais baseados em diversidade humana realista
    TArray<FLinearColor> SkinTones = {
        FLinearColor(0.95f, 0.87f, 0.73f, 1.0f), // Pele clara
        FLinearColor(0.89f, 0.78f, 0.62f, 1.0f), // Pele média-clara
        FLinearColor(0.76f, 0.65f, 0.48f, 1.0f), // Pele média
        FLinearColor(0.65f, 0.52f, 0.37f, 1.0f), // Pele média-escura
        FLinearColor(0.52f, 0.39f, 0.28f, 1.0f), // Pele escura
        FLinearColor(0.41f, 0.28f, 0.19f, 1.0f)  // Pele muito escura
    };
    
    int32 ToneIndex = (int32)(GenerateRandomFloat(Seed) * SkinTones.Num());
    ToneIndex = FMath::Clamp(ToneIndex, 0, SkinTones.Num() - 1);
    
    // Adicionar ligeira variação
    FLinearColor BaseTone = SkinTones[ToneIndex];
    float Variation = 0.05f;
    BaseTone.R = FMath::Clamp(BaseTone.R + GenerateRandomFloat(Seed, -Variation, Variation), 0.0f, 1.0f);
    BaseTone.G = FMath::Clamp(BaseTone.G + GenerateRandomFloat(Seed, -Variation, Variation), 0.0f, 1.0f);
    BaseTone.B = FMath::Clamp(BaseTone.B + GenerateRandomFloat(Seed, -Variation, Variation), 0.0f, 1.0f);
    
    return BaseTone;
}

FLinearColor UCharacterGenerationSystem::GenerateNaturalEyeColor(int32& Seed)
{
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Castanho escuro
        FLinearColor(0.4f, 0.25f, 0.1f, 1.0f),  // Castanho
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),   // Castanho claro
        FLinearColor(0.3f, 0.5f, 0.2f, 1.0f),   // Verde
        FLinearColor(0.2f, 0.4f, 0.7f, 1.0f),   // Azul
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f),   // Cinzento
        FLinearColor(0.15f, 0.08f, 0.03f, 1.0f) // Quase preto
    };
    
    int32 ColorIndex = (int32)(GenerateRandomFloat(Seed) * EyeColors.Num());
    ColorIndex = FMath::Clamp(ColorIndex, 0, EyeColors.Num() - 1);
    
    return EyeColors[ColorIndex];
}

FLinearColor UCharacterGenerationSystem::GenerateNaturalHairColor(int32& Seed)
{
    TArray<FLinearColor> HairColors = {
        FLinearColor(0.05f, 0.02f, 0.01f, 1.0f), // Preto
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),   // Castanho escuro
        FLinearColor(0.4f, 0.25f, 0.15f, 1.0f),  // Castanho
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),    // Castanho claro
        FLinearColor(0.8f, 0.6f, 0.3f, 1.0f),    // Louro escuro
        FLinearColor(0.9f, 0.8f, 0.5f, 1.0f),    // Louro
        FLinearColor(0.7f, 0.3f, 0.1f, 1.0f),    // Ruivo
        FLinearColor(0.6f, 0.6f, 0.6f, 1.0f)     // Grisalho (para mais velhos)
    };
    
    int32 ColorIndex = (int32)(GenerateRandomFloat(Seed) * HairColors.Num());
    ColorIndex = FMath::Clamp(ColorIndex, 0, HairColors.Num() - 1);
    
    return HairColors[ColorIndex];
}

FString UCharacterGenerationSystem::SelectRandomHairStyle(int32& Seed, bool bIsMale)
{
    TArray<FString> MaleStyles = {
        "Tribal_Short_Messy",
        "Tribal_Medium_Wild",
        "Tribal_Long_Braided",
        "Tribal_Mohawk",
        "Tribal_Shaved_Sides",
        "Modern_Academic" // Para o protagonista
    };
    
    TArray<FString> FemaleStyles = {
        "Tribal_Long_Loose",
        "Tribal_Braided_Crown",
        "Tribal_Side_Braid",
        "Tribal_Twisted_Bun",
        "Tribal_Half_Up",
        "Modern_Professional" // Para protagonista feminina
    };
    
    TArray<FString>& Styles = bIsMale ? MaleStyles : FemaleStyles;
    int32 StyleIndex = (int32)(GenerateRandomFloat(Seed) * Styles.Num());
    StyleIndex = FMath::Clamp(StyleIndex, 0, Styles.Num() - 1);
    
    return Styles[StyleIndex];
}

TArray<FString> UCharacterGenerationSystem::GenerateTribalMarkings(int32& Seed, ECharacterArchetype Archetype)
{
    TArray<FString> Markings;
    
    // Markings baseadas no arquétipo
    switch (Archetype)
    {
        case ECharacterArchetype::TribalShaman:
            Markings.Add("Spiritual_Face_Paint");
            Markings.Add("Sacred_Symbols_Arms");
            if (GenerateRandomFloat(Seed) > 0.5f) Markings.Add("Third_Eye_Marking");
            break;
            
        case ECharacterArchetype::TribalWarrior:
            Markings.Add("War_Paint_Face");
            Markings.Add("Victory_Marks_Chest");
            if (GenerateRandomFloat(Seed) > 0.7f) Markings.Add("Battle_Scars_Arms");
            break;
            
        case ECharacterArchetype::TribalElder:
            Markings.Add("Wisdom_Lines_Face");
            Markings.Add("Life_Story_Arms");
            Markings.Add("Elder_Status_Forehead");
            break;
            
        case ECharacterArchetype::TribalCrafter:
            Markings.Add("Craft_Symbols_Hands");
            if (GenerateRandomFloat(Seed) > 0.6f) Markings.Add("Tool_Marks_Arms");
            break;
            
        default:
            // Markings básicas para outros tipos tribais
            if (GenerateRandomFloat(Seed) > 0.5f) Markings.Add("Basic_Tribal_Face");
            if (GenerateRandomFloat(Seed) > 0.7f) Markings.Add("Simple_Arm_Bands");
            break;
    }
    
    return Markings;
}

void UCharacterGenerationSystem::GenerateClothing(FCharacterDefinition& Character, int32& Seed)
{
    // Materiais baseados no arquétipo
    switch (Character.Archetype)
    {
        case ECharacterArchetype::Protagonist:
            Character.Clothing.ClothingMaterials = {"Modern_Fabric", "Leather_Boots", "Metal_Buckles"};
            Character.Clothing.WearLevel = GenerateRandomFloat(Seed, 0.6f, 0.9f); // Desgastado pela viagem temporal
            Character.Clothing.PrimaryColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f); // Tons académicos
            break;
            
        case ECharacterArchetype::TribalWarrior:
            Character.Clothing.ClothingMaterials = {"Animal_Hide", "Bone_Armor", "Sinew_Bindings"};
            Character.Clothing.Ornaments = {"Predator_Teeth", "Claw_Necklace", "Bone_Bracers"};
            Character.Clothing.WearLevel = GenerateRandomFloat(Seed, 0.4f, 0.7f);
            Character.Clothing.PrimaryColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Terra/sangue
            break;
            
        case ECharacterArchetype::TribalShaman:
            Character.Clothing.ClothingMaterials = {"Soft_Pelts", "Feathers", "Plant_Fibers"};
            Character.Clothing.Ornaments = {"Crystal_Pendant", "Feather_Headdress", "Bone_Charms"};
            Character.Clothing.WearLevel = GenerateRandomFloat(Seed, 0.2f, 0.5f);
            Character.Clothing.PrimaryColor = FLinearColor(0.5f, 0.4f, 0.6f, 1.0f); // Tons místicos
            break;
            
        case ECharacterArchetype::TribalElder:
            Character.Clothing.ClothingMaterials = {"Fine_Pelts", "Woven_Fibers", "Polished_Bone"};
            Character.Clothing.Ornaments = {"Status_Medallion", "Wisdom_Staff", "Honor_Beads"};
            Character.Clothing.WearLevel = GenerateRandomFloat(Seed, 0.3f, 0.6f);
            Character.Clothing.PrimaryColor = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f); // Tons ricos
            break;
            
        default:
            Character.Clothing.ClothingMaterials = {"Basic_Hide", "Plant_Fibers"};
            Character.Clothing.WearLevel = GenerateRandomFloat(Seed, 0.5f, 0.8f);
            Character.Clothing.PrimaryColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
            break;
    }
    
    // Cor secundária sempre mais escura que a primária
    Character.Clothing.SecondaryColor = Character.Clothing.PrimaryColor * 0.7f;
    Character.Clothing.SecondaryColor.A = 1.0f;
}

FString UCharacterGenerationSystem::GenerateCharacterName(ECharacterArchetype Archetype, bool bIsMale, int32& Seed)
{
    TArray<FString> TribalMaleNames = {
        "Kael", "Theron", "Drak", "Vex", "Zahn", "Brix", "Tor", "Jax", "Kron", "Rex"
    };
    
    TArray<FString> TribalFemaleNames = {
        "Lyra", "Nyx", "Vera", "Zara", "Kira", "Tess", "Mira", "Lux", "Dara", "Ava"
    };
    
    TArray<FString> ModernMaleNames = {
        "Dr. Marcus", "Dr. James", "Dr. Robert", "Dr. David", "Dr. Michael"
    };
    
    TArray<FString> ModernFemaleNames = {
        "Dr. Sarah", "Dr. Emma", "Dr. Lisa", "Dr. Anna", "Dr. Claire"
    };
    
    TArray<FString>* NameList;
    
    if (Archetype == ECharacterArchetype::Protagonist || Archetype == ECharacterArchetype::TimeTraveler)
    {
        NameList = bIsMale ? &ModernMaleNames : &ModernFemaleNames;
    }
    else
    {
        NameList = bIsMale ? &TribalMaleNames : &TribalFemaleNames;
    }
    
    int32 NameIndex = (int32)(GenerateRandomFloat(Seed) * NameList->Num());
    NameIndex = FMath::Clamp(NameIndex, 0, NameList->Num() - 1);
    
    return (*NameList)[NameIndex];
}