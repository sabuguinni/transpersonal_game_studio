#include "CharacterSystem.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

UCharacterAppearanceComponent::UCharacterAppearanceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MetaHumanMesh = nullptr;
}

void UCharacterAppearanceComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar o componente MetaHuman no actor
    if (AActor* Owner = GetOwner())
    {
        MetaHumanMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    SetupMetaHumanParameters();
}

void UCharacterAppearanceComponent::ApplyCharacterData(const FCharacterData& NewCharacterData)
{
    CharacterData = NewCharacterData;
    
    // Aplicar mesh MetaHuman se especificado
    if (NewCharacterData.MetaHumanMesh.IsValid() && MetaHumanMesh)
    {
        if (USkeletalMesh* LoadedMesh = NewCharacterData.MetaHumanMesh.LoadSynchronous())
        {
            MetaHumanMesh->SetSkeletalMesh(LoadedMesh);
        }
    }
    
    ApplyPhysicalVariations();
    ApplyClothing();
}

void UCharacterAppearanceComponent::GenerateRandomAppearance(ETribalGroup Tribe, ECharacterType Type)
{
    if (UCharacterGenerationSubsystem* GenSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCharacterGenerationSubsystem>())
    {
        FCharacterData GeneratedData = GenSubsystem->GenerateRandomCharacter(Tribe, Type);
        ApplyCharacterData(GeneratedData);
    }
}

void UCharacterAppearanceComponent::ApplyPhysicalVariations()
{
    if (!MetaHumanMesh) return;
    
    const FPhysicalVariation& Traits = CharacterData.PhysicalTraits;
    
    // Aplicar cores
    ApplySkinTone(Traits.SkinTone);
    ApplyHairColor(Traits.HairColor);
    ApplyEyeColor(Traits.EyeColor);
    
    // Aplicar forma corporal
    ApplyBodyShape(Traits.Height, Traits.Weight, Traits.MuscleDefinition);
}

void UCharacterAppearanceComponent::ApplyClothing()
{
    if (!MetaHumanMesh) return;
    
    // Aplicar materiais de roupa baseados no grupo tribal
    for (int32 i = 0; i < CharacterData.ClothingMaterials.Num(); i++)
    {
        if (CharacterData.ClothingMaterials[i].IsValid())
        {
            if (UMaterialInterface* Material = CharacterData.ClothingMaterials[i].LoadSynchronous())
            {
                MetaHumanMesh->SetMaterial(i, Material);
            }
        }
    }
}

void UCharacterAppearanceComponent::SetupMetaHumanParameters()
{
    // Configurar parâmetros base do MetaHuman
    if (MetaHumanMesh)
    {
        // Ativar LOD dinâmico
        MetaHumanMesh->SetForcedLOD(0);
        MetaHumanMesh->bUseBoundsFromMasterPoseComponent = false;
        
        // Configurar qualidade de renderização
        MetaHumanMesh->SetCastShadow(true);
        MetaHumanMesh->SetReceivesDecals(true);
    }
}

void UCharacterAppearanceComponent::ApplySkinTone(FLinearColor SkinColor)
{
    if (!MetaHumanMesh) return;
    
    // Aplicar cor da pele através de parâmetros do material
    for (int32 i = 0; i < MetaHumanMesh->GetNumMaterials(); i++)
    {
        if (UMaterialInstanceDynamic* DynMaterial = MetaHumanMesh->CreateAndSetMaterialInstanceDynamic(i))
        {
            DynMaterial->SetVectorParameterValue(TEXT("Skin_Tone"), SkinColor);
        }
    }
}

void UCharacterAppearanceComponent::ApplyHairColor(FLinearColor HairColor)
{
    if (!MetaHumanMesh) return;
    
    for (int32 i = 0; i < MetaHumanMesh->GetNumMaterials(); i++)
    {
        if (UMaterialInstanceDynamic* DynMaterial = MetaHumanMesh->CreateAndSetMaterialInstanceDynamic(i))
        {
            DynMaterial->SetVectorParameterValue(TEXT("Hair_Color"), HairColor);
        }
    }
}

void UCharacterAppearanceComponent::ApplyEyeColor(FLinearColor EyeColor)
{
    if (!MetaHumanMesh) return;
    
    for (int32 i = 0; i < MetaHumanMesh->GetNumMaterials(); i++)
    {
        if (UMaterialInstanceDynamic* DynMaterial = MetaHumanMesh->CreateAndSetMaterialInstanceDynamic(i))
        {
            DynMaterial->SetVectorParameterValue(TEXT("Eye_Color"), EyeColor);
        }
    }
}

void UCharacterAppearanceComponent::ApplyBodyShape(float Height, float Weight, float Muscle)
{
    if (!MetaHumanMesh) return;
    
    // Aplicar morfologia corporal através de morph targets
    FVector3f Scale = FVector3f(1.0f + (Weight - 1.0f) * 0.2f, 1.0f + (Weight - 1.0f) * 0.2f, Height);
    MetaHumanMesh->SetRelativeScale3D(FVector(Scale));
    
    // Aplicar definição muscular através de parâmetros
    for (int32 i = 0; i < MetaHumanMesh->GetNumMaterials(); i++)
    {
        if (UMaterialInstanceDynamic* DynMaterial = MetaHumanMesh->CreateAndSetMaterialInstanceDynamic(i))
        {
            DynMaterial->SetScalarParameterValue(TEXT("Muscle_Definition"), Muscle);
        }
    }
}

// Character Generation Subsystem Implementation
void UCharacterGenerationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeTribalGenePools();
}

FCharacterData UCharacterGenerationSubsystem::GenerateRandomCharacter(ETribalGroup Tribe, ECharacterType Type)
{
    FCharacterData NewCharacter;
    NewCharacter.CharacterType = Type;
    NewCharacter.TribalGroup = Tribe;
    
    // Gerar nome baseado no tipo e tribo
    switch (Tribe)
    {
        case ETribalGroup::RiverTribe:
            NewCharacter.CharacterName = FString::Printf(TEXT("River_%s_%d"), 
                Type == ECharacterType::Protagonist ? TEXT("Hero") : TEXT("Member"), 
                FMath::RandRange(1, 999));
            break;
        case ETribalGroup::MountainTribe:
            NewCharacter.CharacterName = FString::Printf(TEXT("Mountain_%s_%d"), 
                Type == ECharacterType::Protagonist ? TEXT("Hero") : TEXT("Member"), 
                FMath::RandRange(1, 999));
            break;
        default:
            NewCharacter.CharacterName = FString::Printf(TEXT("Tribal_%s_%d"), 
                Type == ECharacterType::Protagonist ? TEXT("Hero") : TEXT("Member"), 
                FMath::RandRange(1, 999));
            break;
    }
    
    // Gerar idade baseada no tipo
    switch (Type)
    {
        case ECharacterType::Child:
            NewCharacter.Age = FMath::RandRange(8, 16);
            break;
        case ECharacterType::Elder:
            NewCharacter.Age = FMath::RandRange(50, 70);
            break;
        case ECharacterType::Protagonist:
            NewCharacter.Age = FMath::RandRange(30, 45); // Paleontologista experiente
            break;
        default:
            NewCharacter.Age = FMath::RandRange(18, 50);
            break;
    }
    
    // Gerar características físicas baseadas na tribo
    NewCharacter.PhysicalTraits.SkinTone = GenerateTribalSkinTone(Tribe);
    NewCharacter.PhysicalTraits.HairColor = GenerateTribalHairColor(Tribe);
    NewCharacter.PhysicalTraits.EyeColor = GenerateTribalEyeColor(Tribe);
    
    // Gerar variações físicas
    NewCharacter.PhysicalTraits.Height = FMath::FRandRange(0.85f, 1.15f);
    NewCharacter.PhysicalTraits.Weight = FMath::FRandRange(0.8f, 1.2f);
    
    // Definição muscular baseada no tipo
    switch (Type)
    {
        case ECharacterType::Hunter:
            NewCharacter.PhysicalTraits.MuscleDefinition = FMath::FRandRange(0.7f, 1.0f);
            break;
        case ECharacterType::Child:
            NewCharacter.PhysicalTraits.MuscleDefinition = FMath::FRandRange(0.1f, 0.3f);
            break;
        case ECharacterType::Elder:
            NewCharacter.PhysicalTraits.MuscleDefinition = FMath::FRandRange(0.2f, 0.5f);
            break;
        default:
            NewCharacter.PhysicalTraits.MuscleDefinition = FMath::FRandRange(0.4f, 0.8f);
            break;
    }
    
    return NewCharacter;
}

TArray<FCharacterData> UCharacterGenerationSubsystem::GenerateTribalPopulation(ETribalGroup Tribe, int32 PopulationSize)
{
    TArray<FCharacterData> Population;
    
    // Distribuição de tipos na população
    int32 Leaders = FMath::Max(1, PopulationSize / 20);      // 5% líderes
    int32 Shamans = FMath::Max(1, PopulationSize / 25);      // 4% xamãs
    int32 Hunters = PopulationSize / 4;                      // 25% caçadores
    int32 Gatherers = PopulationSize / 3;                    // 33% coletores
    int32 Children = PopulationSize / 5;                     // 20% crianças
    int32 Elders = PopulationSize / 10;                      // 10% anciãos
    int32 Members = PopulationSize - (Leaders + Shamans + Hunters + Gatherers + Children + Elders);
    
    // Gerar cada tipo
    for (int32 i = 0; i < Leaders; i++)
        Population.Add(GenerateRandomCharacter(Tribe, ECharacterType::TribalLeader));
    
    for (int32 i = 0; i < Shamans; i++)
        Population.Add(GenerateRandomCharacter(Tribe, ECharacterType::Shaman));
    
    for (int32 i = 0; i < Hunters; i++)
        Population.Add(GenerateRandomCharacter(Tribe, ECharacterType::Hunter));
    
    for (int32 i = 0; i < Gatherers; i++)
        Population.Add(GenerateRandomCharacter(Tribe, ECharacterType::Gatherer));
    
    for (int32 i = 0; i < Children; i++)
        Population.Add(GenerateRandomCharacter(Tribe, ECharacterType::Child));
    
    for (int32 i = 0; i < Elders; i++)
        Population.Add(GenerateRandomCharacter(Tribe, ECharacterType::Elder));
    
    for (int32 i = 0; i < Members; i++)
        Population.Add(GenerateRandomCharacter(Tribe, ECharacterType::TribalMember));
    
    return Population;
}

void UCharacterGenerationSubsystem::InitializeTribalGenePools()
{
    // Inicializar pools genéticos para cada tribo
    // Cada tribo tem características físicas distintas baseadas no ambiente
    
    // River Tribe - tons mais claros, adaptados à vida aquática
    TArray<FPhysicalVariation> RiverPool;
    for (int32 i = 0; i < 10; i++)
    {
        FPhysicalVariation Variant;
        Variant.SkinTone = FLinearColor(FMath::FRandRange(0.6f, 0.8f), FMath::FRandRange(0.5f, 0.7f), FMath::FRandRange(0.4f, 0.6f), 1.0f);
        Variant.HairColor = FLinearColor(FMath::FRandRange(0.3f, 0.6f), FMath::FRandRange(0.2f, 0.4f), FMath::FRandRange(0.1f, 0.2f), 1.0f);
        RiverPool.Add(Variant);
    }
    TribalGenePool.Add(ETribalGroup::RiverTribe, RiverPool);
    
    // Mountain Tribe - tons médios, estrutura robusta
    TArray<FPhysicalVariation> MountainPool;
    for (int32 i = 0; i < 10; i++)
    {
        FPhysicalVariation Variant;
        Variant.SkinTone = FLinearColor(FMath::FRandRange(0.5f, 0.7f), FMath::FRandRange(0.4f, 0.6f), FMath::FRandRange(0.3f, 0.5f), 1.0f);
        Variant.HairColor = FLinearColor(FMath::FRandRange(0.1f, 0.3f), FMath::FRandRange(0.05f, 0.2f), FMath::FRandRange(0.02f, 0.1f), 1.0f);
        MountainPool.Add(Variant);
    }
    TribalGenePool.Add(ETribalGroup::MountainTribe, MountainPool);
    
    // Continuar para outras tribos...
}

FLinearColor UCharacterGenerationSubsystem::GenerateTribalSkinTone(ETribalGroup Tribe)
{
    switch (Tribe)
    {
        case ETribalGroup::RiverTribe:
            return FLinearColor(FMath::FRandRange(0.6f, 0.8f), FMath::FRandRange(0.5f, 0.7f), FMath::FRandRange(0.4f, 0.6f), 1.0f);
        case ETribalGroup::MountainTribe:
            return FLinearColor(FMath::FRandRange(0.5f, 0.7f), FMath::FRandRange(0.4f, 0.6f), FMath::FRandRange(0.3f, 0.5f), 1.0f);
        case ETribalGroup::ForestTribe:
            return FLinearColor(FMath::FRandRange(0.4f, 0.6f), FMath::FRandRange(0.3f, 0.5f), FMath::FRandRange(0.25f, 0.45f), 1.0f);
        case ETribalGroup::PlainsTribe:
            return FLinearColor(FMath::FRandRange(0.7f, 0.9f), FMath::FRandRange(0.6f, 0.8f), FMath::FRandRange(0.5f, 0.7f), 1.0f);
        case ETribalGroup::CoastalTribe:
            return FLinearColor(FMath::FRandRange(0.55f, 0.75f), FMath::FRandRange(0.45f, 0.65f), FMath::FRandRange(0.35f, 0.55f), 1.0f);
        default:
            return FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
    }
}

FLinearColor UCharacterGenerationSubsystem::GenerateTribalHairColor(ETribalGroup Tribe)
{
    // Variações de cor de cabelo baseadas na tribo
    float BaseR = FMath::FRandRange(0.1f, 0.4f);
    float BaseG = FMath::FRandRange(0.05f, 0.3f);
    float BaseB = FMath::FRandRange(0.02f, 0.15f);
    
    return FLinearColor(BaseR, BaseG, BaseB, 1.0f);
}

FLinearColor UCharacterGenerationSubsystem::GenerateTribalEyeColor(ETribalGroup Tribe)
{
    // Cores de olho variadas mas realistas
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f),  // Marrom escuro
        FLinearColor(0.4f, 0.3f, 0.1f, 1.0f),   // Marrom
        FLinearColor(0.6f, 0.5f, 0.2f, 1.0f),   // Avelã
        FLinearColor(0.3f, 0.4f, 0.6f, 1.0f),   // Azul acinzentado
        FLinearColor(0.2f, 0.5f, 0.3f, 1.0f)    // Verde
    };
    
    return EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
}