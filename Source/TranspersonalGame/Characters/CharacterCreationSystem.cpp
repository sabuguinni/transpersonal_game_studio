#include "CharacterCreationSystem.h"
#include "Engine/DataTable.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"

// Inicialização do contador de IDs únicos
int32 UCharacterCreationSystem::NextUniqueID = 1;

UCharacterCreationSystem::UCharacterCreationSystem()
{
    // Configuração das variações de tom de pele para diversidade
    SkinToneVariations.Add(FLinearColor(0.95f, 0.87f, 0.73f, 1.0f)); // Pele clara
    SkinToneVariations.Add(FLinearColor(0.85f, 0.72f, 0.56f, 1.0f)); // Pele média-clara
    SkinToneVariations.Add(FLinearColor(0.76f, 0.63f, 0.48f, 1.0f)); // Pele média
    SkinToneVariations.Add(FLinearColor(0.65f, 0.52f, 0.39f, 1.0f)); // Pele média-escura
    SkinToneVariations.Add(FLinearColor(0.54f, 0.42f, 0.31f, 1.0f)); // Pele escura
    SkinToneVariations.Add(FLinearColor(0.43f, 0.33f, 0.24f, 1.0f)); // Pele muito escura
}

FCharacterDefinition UCharacterCreationSystem::GenerateRandomCharacter(ECharacterArchetype DesiredArchetype)
{
    FCharacterDefinition NewCharacter;
    
    // Atribuir ID único
    NewCharacter.UniqueID = NextUniqueID++;
    
    // Determinar arquétipo
    if (DesiredArchetype == ECharacterArchetype::None)
    {
        // Escolher arquétipo aleatório
        int32 RandomArchetype = FMath::RandRange(1, 9); // Excluindo None (0)
        NewCharacter.Archetype = static_cast<ECharacterArchetype>(RandomArchetype);
    }
    else
    {
        NewCharacter.Archetype = DesiredArchetype;
    }
    
    // Gerar características básicas
    NewCharacter.bIsMale = FMath::RandBool();
    NewCharacter.Age = GenerateAgeForArchetype(NewCharacter.Archetype);
    NewCharacter.CharacterName = GenerateNameForCharacter(NewCharacter.bIsMale, NewCharacter.Archetype);
    
    // Gerar características físicas baseadas no arquétipo
    NewCharacter.PhysicalTraits = GenerateTraitsForArchetype(NewCharacter.Archetype);
    
    // Aplicar variação única para evitar clones
    FCharacterPhysicalTraits UniqueVariation = GenerateUniqueVariation(NewCharacter.UniqueID);
    BlendPhysicalTraits(NewCharacter.PhysicalTraits, UniqueVariation, 0.3f);
    
    // Gerar roupas apropriadas para o arquétipo
    NewCharacter.Clothing = GenerateClothingForArchetype(NewCharacter.Archetype);
    
    // Definir caminho do asset MetaHuman (será configurado durante a aplicação)
    NewCharacter.MetaHumanAssetPath = FString::Printf(TEXT("/Game/MetaHumans/Generated/Character_%d"), NewCharacter.UniqueID);
    
    return NewCharacter;
}

FCharacterDefinition UCharacterCreationSystem::CreateProtagonist()
{
    FCharacterDefinition Protagonist;
    
    Protagonist.UniqueID = 0; // ID especial para o protagonista
    Protagonist.CharacterName = TEXT("Dr. Marcus Reed"); // Nome temporário
    Protagonist.Archetype = ECharacterArchetype::None; // Protagonista não é um NPC tribal
    Protagonist.bIsMale = true;
    Protagonist.Age = 35;
    
    // Características físicas do paleontologista moderno
    Protagonist.PhysicalTraits.FaceWidth = 0.5f;
    Protagonist.PhysicalTraits.FaceHeight = 0.55f;
    Protagonist.PhysicalTraits.EyeSize = 0.6f; // Olhos expressivos, inteligentes
    Protagonist.PhysicalTraits.NoseSize = 0.5f;
    Protagonist.PhysicalTraits.MouthSize = 0.45f;
    
    Protagonist.PhysicalTraits.Height = 0.6f; // Altura média-alta
    Protagonist.PhysicalTraits.Weight = 0.4f; // Físico acadêmico, não muito musculoso
    Protagonist.PhysicalTraits.MuscleDefinition = 0.3f;
    
    // Tom de pele de pessoa que trabalha principalmente em ambientes internos
    Protagonist.PhysicalTraits.SkinTone = FLinearColor(0.88f, 0.75f, 0.62f, 1.0f);
    Protagonist.PhysicalTraits.SkinRoughness = 0.2f; // Pele relativamente suave
    Protagonist.PhysicalTraits.WeatheringLevel = 0.1f; // Pouca exposição aos elementos
    
    // Roupas modernas (que se tornarão desgastadas ao longo do jogo)
    Protagonist.Clothing.PrimaryMaterial = TEXT("Modern Fabric");
    Protagonist.Clothing.ClothingColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f); // Azul denim
    Protagonist.Clothing.WearLevel = 0.1f; // Roupas inicialmente em bom estado
    
    // Acessórios de paleontologista
    Protagonist.Clothing.Accessories.Add(TEXT("Field Notebook"));
    Protagonist.Clothing.Accessories.Add(TEXT("Magnifying Glass"));
    Protagonist.Clothing.Accessories.Add(TEXT("Work Gloves"));
    
    Protagonist.MetaHumanAssetPath = TEXT("/Game/MetaHumans/Protagonist/Dr_Marcus_Reed");
    
    return Protagonist;
}

FCharacterPhysicalTraits UCharacterCreationSystem::GenerateTraitsForArchetype(ECharacterArchetype Archetype)
{
    FCharacterPhysicalTraits Traits;
    
    switch (Archetype)
    {
        case ECharacterArchetype::TribalLeader:
            // Líder tribal: presença imponente, bem nutrido
            Traits.Height = FMath::RandRange(0.6f, 0.8f);
            Traits.Weight = FMath::RandRange(0.5f, 0.7f);
            Traits.MuscleDefinition = FMath::RandRange(0.6f, 0.8f);
            Traits.FaceWidth = FMath::RandRange(0.5f, 0.7f);
            Traits.WeatheringLevel = FMath::RandRange(0.4f, 0.6f);
            break;
            
        case ECharacterArchetype::Hunter:
            // Caçador: físico atlético, marcas de exposição
            Traits.Height = FMath::RandRange(0.5f, 0.7f);
            Traits.Weight = FMath::RandRange(0.3f, 0.5f);
            Traits.MuscleDefinition = FMath::RandRange(0.7f, 0.9f);
            Traits.WeatheringLevel = FMath::RandRange(0.6f, 0.8f);
            break;
            
        case ECharacterArchetype::Gatherer:
            // Coletor: físico resistente mas não muito musculoso
            Traits.Height = FMath::RandRange(0.4f, 0.6f);
            Traits.Weight = FMath::RandRange(0.4f, 0.6f);
            Traits.MuscleDefinition = FMath::RandRange(0.4f, 0.6f);
            Traits.WeatheringLevel = FMath::RandRange(0.5f, 0.7f);
            break;
            
        case ECharacterArchetype::Shaman:
            // Xamã: aparência mística, possivelmente mais velho
            Traits.Height = FMath::RandRange(0.4f, 0.6f);
            Traits.Weight = FMath::RandRange(0.3f, 0.5f);
            Traits.MuscleDefinition = FMath::RandRange(0.2f, 0.4f);
            Traits.EyeSize = FMath::RandRange(0.6f, 0.8f); // Olhos penetrantes
            Traits.WeatheringLevel = FMath::RandRange(0.7f, 0.9f);
            break;
            
        case ECharacterArchetype::Elder:
            // Ancião: sinais de idade, sabedoria
            Traits.Height = FMath::RandRange(0.3f, 0.5f);
            Traits.Weight = FMath::RandRange(0.3f, 0.5f);
            Traits.MuscleDefinition = FMath::RandRange(0.1f, 0.3f);
            Traits.WeatheringLevel = FMath::RandRange(0.8f, 1.0f);
            break;
            
        case ECharacterArchetype::Child:
            // Criança: características juvenis
            Traits.Height = FMath::RandRange(0.2f, 0.4f);
            Traits.Weight = FMath::RandRange(0.2f, 0.4f);
            Traits.MuscleDefinition = FMath::RandRange(0.1f, 0.2f);
            Traits.EyeSize = FMath::RandRange(0.7f, 0.9f); // Olhos grandes
            Traits.WeatheringLevel = FMath::RandRange(0.1f, 0.3f);
            break;
            
        default:
            // Valores padrão para outros arquétipos
            Traits.Height = FMath::RandRange(0.4f, 0.6f);
            Traits.Weight = FMath::RandRange(0.4f, 0.6f);
            Traits.MuscleDefinition = FMath::RandRange(0.4f, 0.6f);
            Traits.WeatheringLevel = FMath::RandRange(0.4f, 0.6f);
            break;
    }
    
    // Características faciais comuns
    Traits.FaceHeight = FMath::RandRange(0.4f, 0.6f);
    Traits.NoseSize = FMath::RandRange(0.4f, 0.6f);
    Traits.MouthSize = FMath::RandRange(0.4f, 0.6f);
    
    // Selecionar tom de pele aleatório da paleta de diversidade
    if (SkinToneVariations.Num() > 0)
    {
        int32 RandomSkinIndex = FMath::RandRange(0, SkinToneVariations.Num() - 1);
        Traits.SkinTone = SkinToneVariations[RandomSkinIndex];
    }
    
    // Rugosidade da pele baseada na exposição aos elementos
    Traits.SkinRoughness = FMath::Clamp(0.3f + (Traits.WeatheringLevel * 0.4f), 0.2f, 0.8f);
    
    return Traits;
}

FPrehistoricClothing UCharacterCreationSystem::GenerateClothingForArchetype(ECharacterArchetype Archetype)
{
    FPrehistoricClothing Clothing;
    
    // Materiais base para roupas pré-históricas
    TArray<FString> BaseMaterials = {
        TEXT("Mammoth Hide"),
        TEXT("Deer Skin"),
        TEXT("Bear Fur"),
        TEXT("Plant Fibers"),
        TEXT("Woven Grass"),
        TEXT("Fish Skin")
    };
    
    Clothing.PrimaryMaterial = BaseMaterials[FMath::RandRange(0, BaseMaterials.Num() - 1)];
    
    // Cores naturais baseadas em materiais disponíveis no período
    TArray<FLinearColor> NaturalColors = {
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f), // Marrom couro
        FLinearColor(0.5f, 0.3f, 0.1f, 1.0f), // Marrom escuro
        FLinearColor(0.8f, 0.7f, 0.5f, 1.0f), // Bege natural
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f), // Marrom muito escuro
        FLinearColor(0.4f, 0.5f, 0.3f, 1.0f)  // Verde musgo
    };
    
    Clothing.ClothingColor = NaturalColors[FMath::RandRange(0, NaturalColors.Num() - 1)];
    
    // Configurar acessórios baseados no arquétipo
    switch (Archetype)
    {
        case ECharacterArchetype::TribalLeader:
            Clothing.Accessories.Add(TEXT("Feather Headdress"));
            Clothing.Accessories.Add(TEXT("Bone Necklace"));
            Clothing.Accessories.Add(TEXT("Ceremonial Markings"));
            Clothing.WearLevel = FMath::RandRange(0.1f, 0.3f); // Roupas bem cuidadas
            break;
            
        case ECharacterArchetype::Hunter:
            Clothing.Accessories.Add(TEXT("Claw Necklace"));
            Clothing.Accessories.Add(TEXT("Hunting Pouch"));
            Clothing.Accessories.Add(TEXT("Camouflage Paint"));
            Clothing.WearLevel = FMath::RandRange(0.4f, 0.7f); // Roupas desgastadas pelo uso
            break;
            
        case ECharacterArchetype::Shaman:
            Clothing.Accessories.Add(TEXT("Ritual Bones"));
            Clothing.Accessories.Add(TEXT("Sacred Stones"));
            Clothing.Accessories.Add(TEXT("Spirit Markings"));
            Clothing.Accessories.Add(TEXT("Herb Pouch"));
            Clothing.WearLevel = FMath::RandRange(0.3f, 0.6f);
            break;
            
        case ECharacterArchetype::Craftsperson:
            Clothing.Accessories.Add(TEXT("Tool Belt"));
            Clothing.Accessories.Add(TEXT("Material Samples"));
            Clothing.WearLevel = FMath::RandRange(0.5f, 0.8f); // Muito desgastadas pelo trabalho
            break;
            
        default:
            // Acessórios básicos para outros arquétipos
            if (FMath::RandBool())
                Clothing.Accessories.Add(TEXT("Simple Bone Ornament"));
            if (FMath::RandBool())
                Clothing.Accessories.Add(TEXT("Woven Bracelet"));
            Clothing.WearLevel = FMath::RandRange(0.3f, 0.6f);
            break;
    }
    
    return Clothing;
}

FCharacterPhysicalTraits UCharacterCreationSystem::GenerateUniqueVariation(int32 SeedValue)
{
    // Usar o seed para gerar variações consistentes mas únicas
    FRandomStream RandomStream(SeedValue);
    
    FCharacterPhysicalTraits Variation;
    
    // Gerar pequenas variações em todas as características
    Variation.FaceWidth = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    Variation.FaceHeight = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    Variation.EyeSize = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    Variation.NoseSize = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    Variation.MouthSize = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    
    Variation.Height = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    Variation.Weight = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    Variation.MuscleDefinition = RandomStream.FRandRange(-PhysicalVariationRange, PhysicalVariationRange);
    
    // Variação sutil no tom de pele
    float SkinVariation = RandomStream.FRandRange(-0.1f, 0.1f);
    Variation.SkinTone = FLinearColor(SkinVariation, SkinVariation, SkinVariation, 0.0f);
    
    Variation.WeatheringLevel = RandomStream.FRandRange(-WeatheringVariationRange, WeatheringVariationRange);
    
    return Variation;
}

FString UCharacterCreationSystem::GenerateNameForCharacter(bool bIsMale, ECharacterArchetype Archetype)
{
    // Nomes pré-históricos inspirados em elementos naturais
    TArray<FString> MaleNames = {
        TEXT("Kael"), TEXT("Thane"), TEXT("Bron"), TEXT("Drak"), TEXT("Gor"),
        TEXT("Hark"), TEXT("Jorn"), TEXT("Kron"), TEXT("Lok"), TEXT("Mak"),
        TEXT("Nar"), TEXT("Orn"), TEXT("Pak"), TEXT("Rok"), TEXT("Sek"),
        TEXT("Tak"), TEXT("Ur"), TEXT("Vek"), TEXT("Wok"), TEXT("Zak")
    };
    
    TArray<FString> FemaleNames = {
        TEXT("Ayla"), TEXT("Bira"), TEXT("Cira"), TEXT("Dara"), TEXT("Ela"),
        TEXT("Fira"), TEXT("Gira"), TEXT("Hira"), TEXT("Ira"), TEXT("Jira"),
        TEXT("Kira"), TEXT("Lara"), TEXT("Mira"), TEXT("Nira"), TEXT("Ora"),
        TEXT("Pira"), TEXT("Qira"), TEXT("Rira"), TEXT("Sira"), TEXT("Tira")
    };
    
    // Sufixos baseados no arquétipo
    TMap<ECharacterArchetype, FString> ArchetypeSuffixes = {
        {ECharacterArchetype::TribalLeader, TEXT(" the Great")},
        {ECharacterArchetype::Hunter, TEXT(" the Swift")},
        {ECharacterArchetype::Shaman, TEXT(" the Wise")},
        {ECharacterArchetype::Elder, TEXT(" the Ancient")},
        {ECharacterArchetype::Warrior, TEXT(" the Strong")}
    };
    
    FString BaseName;
    if (bIsMale && MaleNames.Num() > 0)
    {
        BaseName = MaleNames[FMath::RandRange(0, MaleNames.Num() - 1)];
    }
    else if (!bIsMale && FemaleNames.Num() > 0)
    {
        BaseName = FemaleNames[FMath::RandRange(0, FemaleNames.Num() - 1)];
    }
    else
    {
        BaseName = TEXT("Unknown");
    }
    
    // Adicionar sufixo se apropriado para o arquétipo
    if (ArchetypeSuffixes.Contains(Archetype))
    {
        BaseName += ArchetypeSuffixes[Archetype];
    }
    
    return BaseName;
}

int32 UCharacterCreationSystem::GenerateAgeForArchetype(ECharacterArchetype Archetype)
{
    switch (Archetype)
    {
        case ECharacterArchetype::Child:
            return FMath::RandRange(8, 15);
        case ECharacterArchetype::Elder:
            return FMath::RandRange(50, 70);
        case ECharacterArchetype::TribalLeader:
            return FMath::RandRange(35, 55);
        case ECharacterArchetype::Shaman:
            return FMath::RandRange(40, 65);
        default:
            return FMath::RandRange(18, 45);
    }
}

void UCharacterCreationSystem::BlendPhysicalTraits(FCharacterPhysicalTraits& BaseTraits, const FCharacterPhysicalTraits& Variation, float BlendWeight)
{
    BaseTraits.FaceWidth = FMath::Clamp(BaseTraits.FaceWidth + (Variation.FaceWidth * BlendWeight), 0.0f, 1.0f);
    BaseTraits.FaceHeight = FMath::Clamp(BaseTraits.FaceHeight + (Variation.FaceHeight * BlendWeight), 0.0f, 1.0f);
    BaseTraits.EyeSize = FMath::Clamp(BaseTraits.EyeSize + (Variation.EyeSize * BlendWeight), 0.0f, 1.0f);
    BaseTraits.NoseSize = FMath::Clamp(BaseTraits.NoseSize + (Variation.NoseSize * BlendWeight), 0.0f, 1.0f);
    BaseTraits.MouthSize = FMath::Clamp(BaseTraits.MouthSize + (Variation.MouthSize * BlendWeight), 0.0f, 1.0f);
    
    BaseTraits.Height = FMath::Clamp(BaseTraits.Height + (Variation.Height * BlendWeight), 0.0f, 1.0f);
    BaseTraits.Weight = FMath::Clamp(BaseTraits.Weight + (Variation.Weight * BlendWeight), 0.0f, 1.0f);
    BaseTraits.MuscleDefinition = FMath::Clamp(BaseTraits.MuscleDefinition + (Variation.MuscleDefinition * BlendWeight), 0.0f, 1.0f);
    
    // Blend de cor de pele
    BaseTraits.SkinTone.R = FMath::Clamp(BaseTraits.SkinTone.R + (Variation.SkinTone.R * BlendWeight), 0.0f, 1.0f);
    BaseTraits.SkinTone.G = FMath::Clamp(BaseTraits.SkinTone.G + (Variation.SkinTone.G * BlendWeight), 0.0f, 1.0f);
    BaseTraits.SkinTone.B = FMath::Clamp(BaseTraits.SkinTone.B + (Variation.SkinTone.B * BlendWeight), 0.0f, 1.0f);
    
    BaseTraits.WeatheringLevel = FMath::Clamp(BaseTraits.WeatheringLevel + (Variation.WeatheringLevel * BlendWeight), 0.0f, 1.0f);
}