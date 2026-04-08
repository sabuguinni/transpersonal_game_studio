#include "CharacterArtistManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "MetaHumanCharacterComponent.h"
#include "MetaHumanCharacterGenerator.h"
#include "CharacterArchetypeSystem.h"
#include "TranspersonalCharacterBase.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

void UCharacterArtistManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Character Artist Manager initialized - Ready to create authentic humans"));
    
    // Initialize diversity tracking
    EthnicityCount.Empty();
    AgeGroupCount.Empty();
    BuildTypeCount.Empty();
    
    // Set up default archetype counts
    AgeGroupCount.Add(ECharacterAgeGroup::Young, 0);
    AgeGroupCount.Add(ECharacterAgeGroup::Adult, 0);
    AgeGroupCount.Add(ECharacterAgeGroup::MiddleAged, 0);
    AgeGroupCount.Add(ECharacterAgeGroup::Elder, 0);
    
    BuildTypeCount.Add(ECharacterBuild::Slim, 0);
    BuildTypeCount.Add(ECharacterBuild::Average, 0);
    BuildTypeCount.Add(ECharacterBuild::Athletic, 0);
    BuildTypeCount.Add(ECharacterBuild::Heavy, 0);
    
    // Load archetype data asset
    if (!ArchetypeDataAsset.IsNull())
    {
        ArchetypeDataAsset.LoadSynchronous();
        UE_LOG(LogTemp, Log, TEXT("Archetype data asset loaded successfully"));
    }
    
    TotalCharactersCreated = 0;
    TotalQualityScore = 0.0f;
}

void UCharacterArtistManager::Deinitialize()
{
    // Clean up character registry
    RegisteredCharacters.Empty();
    NamedCharacters.Empty();
    CharactersByArchetype.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Character Artist Manager deinitialized - %d characters created this session"), TotalCharactersCreated);
    
    Super::Deinitialize();
}

ACharacter* UCharacterArtistManager::CreateCharacterFromArchetype(ECharacterArchetype ArchetypeType, const FVector& SpawnLocation, const FRotator& SpawnRotation, bool bRandomizeTraits)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create character - no valid world"));
        return nullptr;
    }
    
    // Get archetype data
    UCharacterArchetypeDataAsset* ArchetypeData = ArchetypeDataAsset.LoadSynchronous();
    if (!ArchetypeData)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create character - archetype data asset not loaded"));
        return nullptr;
    }
    
    FCharacterArchetypeData* ArchetypeInfo = ArchetypeData->GetArchetypeData(ArchetypeType);
    if (!ArchetypeInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot create character - archetype %d not found"), (int32)ArchetypeType);
        return nullptr;
    }
    
    // Determine character class to spawn
    TSubclassOf<ACharacter> CharacterClass = BaseCharacterClass;
    if (!CharacterClass)
    {
        CharacterClass = ATranspersonalCharacterBase::StaticClass();
    }
    
    // Spawn character
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ACharacter* NewCharacter = GetWorld()->SpawnActor<ACharacter>(CharacterClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (!NewCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn character actor"));
        return nullptr;
    }
    
    // Apply diversity if needed
    FCharacterArchetypeData FinalArchetypeData = *ArchetypeInfo;
    if (bRandomizeTraits && ShouldCreateDiverseVariant(FinalArchetypeData))
    {
        FinalArchetypeData = CreateDiverseVariant(FinalArchetypeData);
    }
    
    // Set up character components and appearance\n    SetupCharacterMeshComponents(NewCharacter, FinalArchetypeData);
    ApplyCharacterCustomization(NewCharacter, FinalArchetypeData);
    SetupCharacterMaterials(NewCharacter, FinalArchetypeData);
    ApplyClothingAndAccessories(NewCharacter, FinalArchetypeData);
    
    // Register character
    RegisterCharacter(NewCharacter);
    
    // Update statistics
    TotalCharactersCreated++;
    TotalQualityScore += DefaultCharacterQuality;
    
    // Update diversity tracking
    AgeGroupCount[FinalArchetypeData.AgeGroup]++;
    BuildTypeCount[FinalArchetypeData.BuildType]++;
    
    UE_LOG(LogTemp, Log, TEXT("Created character from archetype %s at location %s"), 
           *FinalArchetypeData.ArchetypeName, 
           *SpawnLocation.ToString());
    
    return NewCharacter;
}

ACharacter* UCharacterArtistManager::CreateUniqueNamedCharacter(const FString& CharacterName, ECharacterArchetype BaseArchetype, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
    // Check if character with this name already exists
    if (NamedCharacters.Contains(CharacterName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Character with name '%s' already exists"), *CharacterName);
        return NamedCharacters[CharacterName].Get();
    }
    
    // Create base character
    ACharacter* NewCharacter = CreateCharacterFromArchetype(BaseArchetype, SpawnLocation, SpawnRotation, true);
    if (!NewCharacter)
    {
        return nullptr;
    }
    
    // Add to named characters registry
    NamedCharacters.Add(CharacterName, NewCharacter);
    
    // Apply unique character history
    AddCharacterHistory(NewCharacter, FString::Printf(TEXT("Unique individual: %s"), *CharacterName));
    
    UE_LOG(LogTemp, Log, TEXT("Created unique named character: %s"), *CharacterName);
    
    return NewCharacter;
}

void UCharacterArtistManager::PopulateAreaWithCharacters(const FVector& CenterLocation, float Radius, int32 MinCharacters, int32 MaxCharacters, TArray<ECharacterArchetype> AllowedArchetypes)
{
    if (AllowedArchetypes.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot populate area - no allowed archetypes specified"));
        return;
    }
    
    int32 CharactersToSpawn = FMath::RandRange(MinCharacters, MaxCharacters);
    TArray<ACharacter*> SpawnedCharacters;
    
    for (int32 i = 0; i < CharactersToSpawn; i++)
    {
        // Select random archetype
        ECharacterArchetype SelectedArchetype = AllowedArchetypes[FMath::RandRange(0, AllowedArchetypes.Num() - 1)];
        
        // Generate random spawn location within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        // Ensure we're within the circle
        if (RandomOffset.Size2D() > Radius)
        {
            RandomOffset = RandomOffset.GetSafeNormal2D() * FMath::RandRange(0.0f, Radius);
        }
        
        FVector SpawnLocation = CenterLocation + RandomOffset;
        FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        ACharacter* NewCharacter = CreateCharacterFromArchetype(SelectedArchetype, SpawnLocation, SpawnRotation, true);
        if (NewCharacter)
        {
            SpawnedCharacters.Add(NewCharacter);
        }
    }
    
    // Ensure diversity in the spawned group
    EnsureCharacterDiversity(SpawnedCharacters);
    
    UE_LOG(LogTemp, Log, TEXT("Populated area with %d characters (requested %d-%d)"), 
           SpawnedCharacters.Num(), MinCharacters, MaxCharacters);
}

void UCharacterArtistManager::EnsureCharacterDiversity(TArray<ACharacter*>& Characters)
{
    if (Characters.Num() < 2)
    {
        return; // Not enough characters to diversify
    }
    
    // Count current diversity metrics
    TMap<ECharacterAgeGroup, int32> LocalAgeCount;
    TMap<ECharacterBuild, int32> LocalBuildCount;
    
    for (ACharacter* Character : Characters)
    {
        // Get character's MetaHuman component to read current traits
        UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
        if (MetaHumanComp)
        {
            // This would need to be implemented to read current character traits
            // For now, we'll apply some basic diversity rules
        }
    }
    
    // Apply diversity adjustments
    int32 CharactersToModify = FMath::CeilToInt(Characters.Num() * DiversityTargetPercentage);
    
    for (int32 i = 0; i < CharactersToModify && i < Characters.Num(); i++)
    {
        ACharacter* Character = Characters[i];
        if (Character)
        {
            // Apply some random variation to ensure uniqueness
            ApplyEnvironmentalAdaptation(Character, FMath::RandRange(0.1f, 1.0f));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied diversity to %d out of %d characters"), CharactersToModify, Characters.Num());
}

void UCharacterArtistManager::ApplyEnvironmentalAdaptation(ACharacter* Character, float TimeInEnvironment)
{
    if (!Character)
    {
        return;
    }
    
    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp)
    {
        return;
    }
    
    // Calculate adaptation effects based on time in environment
    float DirtLevel = FMath::Clamp(TimeInEnvironment * 0.3f, 0.0f, 0.8f);
    float WearLevel = FMath::Clamp(TimeInEnvironment * 0.2f, 0.0f, 0.6f);
    float ScarLevel = FMath::Clamp(TimeInEnvironment * 0.1f, 0.0f, 0.3f);
    
    // Apply visual changes
    AddVisualDetails(Character, {
        FString::Printf(TEXT("Dirt level: %.2f"), DirtLevel),
        FString::Printf(TEXT("Clothing wear: %.2f"), WearLevel),
        FString::Printf(TEXT("Battle scars: %.2f"), ScarLevel)
    });
    
    UE_LOG(LogTemp, Log, TEXT("Applied environmental adaptation to character (time: %.2f)"), TimeInEnvironment);
}

void UCharacterArtistManager::ApplySurvivalWear(ACharacter* Character, ESurvivalCondition Condition)
{
    if (!Character)
    {
        return;
    }
    
    FString ConditionDescription;
    float WearMultiplier = 1.0f;
    
    switch (Condition)
    {
    case ESurvivalCondition::Pristine:
        ConditionDescription = TEXT("Pristine condition - recently arrived");
        WearMultiplier = 0.0f;
        break;
    case ESurvivalCondition::LightWear:
        ConditionDescription = TEXT("Light wear from basic survival");
        WearMultiplier = 0.2f;
        break;
    case ESurvivalCondition::ModerateWear:
        ConditionDescription = TEXT("Moderate wear from extended survival");
        WearMultiplier = 0.5f;
        break;
    case ESurvivalCondition::HeavyWear:
        ConditionDescription = TEXT("Heavy wear from harsh conditions");
        WearMultiplier = 0.8f;
        break;
    case ESurvivalCondition::Extreme:
        ConditionDescription = TEXT("Extreme wear - barely holding together");
        WearMultiplier = 1.0f;
        break;
    }
    
    AddCharacterHistory(Character, ConditionDescription);
    ApplyWeatheringEffects(Character, WearMultiplier);
    
    UE_LOG(LogTemp, Log, TEXT("Applied survival wear condition: %s"), *ConditionDescription);
}

void UCharacterArtistManager::AddCharacterHistory(ACharacter* Character, const FString& HistoryDescription)
{
    if (!Character)
    {
        return;
    }
    
    // Store history in character's metadata or component
    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (MetaHumanComp)
    {
        // This would be implemented to store character history
        UE_LOG(LogTemp, Log, TEXT("Added character history: %s"), *HistoryDescription);
    }
}

void UCharacterArtistManager::ApplyEmotionalState(ACharacter* Character, const FString& EmotionalState)
{
    if (!Character)
    {
        return;
    }
    
    // Apply facial expression and posture changes based on emotional state
    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (MetaHumanComp)
    {
        // This would be implemented to modify facial expressions
        UE_LOG(LogTemp, Log, TEXT("Applied emotional state: %s"), *EmotionalState);
    }
}

void UCharacterArtistManager::OptimizeCharacterLODs(TArray<ACharacter*>& Characters, const FVector& ViewerLocation)
{
    for (ACharacter* Character : Characters)
    {
        if (Character)
        {
            float Distance = FVector::Dist(Character->GetActorLocation(), ViewerLocation);
            UpdateCharacterLOD(Character, Distance);
        }
    }
}

void UCharacterArtistManager::SetCharacterQualityLevel(ACharacter* Character, int32 QualityLevel)
{
    if (!Character)
    {
        return;
    }
    
    QualityLevel = FMath::Clamp(QualityLevel, 0, 3);
    
    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (MetaHumanComp)
    {
        // This would be implemented to adjust character quality settings
        UE_LOG(LogTemp, Log, TEXT("Set character quality level to %d"), QualityLevel);
    }
}

void UCharacterArtistManager::RegisterCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    RegisteredCharacters.AddUnique(Character);
    
    // Add to archetype-based registry if we can determine the archetype
    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (MetaHumanComp)
    {
        // This would read the character's archetype and add to appropriate array
    }
    
    UE_LOG(LogTemp, Log, TEXT("Registered character - total count: %d"), RegisteredCharacters.Num());
}

void UCharacterArtistManager::UnregisterCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    RegisteredCharacters.RemoveSingle(Character);
    
    // Remove from named characters if present
    for (auto& NamedPair : NamedCharacters)
    {
        if (NamedPair.Value == Character)
        {
            NamedCharacters.Remove(NamedPair.Key);
            break;
        }
    }
    
    // Remove from archetype arrays
    for (auto& ArchetypePair : CharactersByArchetype)
    {
        ArchetypePair.Value.RemoveSingle(Character);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered character - remaining count: %d"), RegisteredCharacters.Num());
}

TArray<ACharacter*> UCharacterArtistManager::GetCharactersByArchetype(ECharacterArchetype ArchetypeType)
{
    TArray<ACharacter*> Result;
    
    if (CharactersByArchetype.Contains(ArchetypeType))
    {
        for (TWeakObjectPtr<ACharacter> WeakChar : CharactersByArchetype[ArchetypeType])
        {
            if (WeakChar.IsValid())
            {
                Result.Add(WeakChar.Get());
            }
        }
    }
    
    return Result;
}

ACharacter* UCharacterArtistManager::GetCharacterByName(const FString& CharacterName)
{
    if (NamedCharacters.Contains(CharacterName))
    {
        return NamedCharacters[CharacterName].Get();
    }
    
    return nullptr;
}

int32 UCharacterArtistManager::GetTotalCharacterCount() const
{
    // Clean up invalid references
    int32 ValidCount = 0;
    for (const TWeakObjectPtr<ACharacter>& WeakChar : RegisteredCharacters)
    {
        if (WeakChar.IsValid())
        {
            ValidCount++;
        }
    }
    
    return ValidCount;
}

float UCharacterArtistManager::GetAverageCharacterQuality() const
{
    if (TotalCharactersCreated == 0)
    {
        return 0.0f;
    }
    
    return TotalQualityScore / TotalCharactersCreated;
}

TMap<ECharacterArchetype, int32> UCharacterArtistManager::GetArchetypeDistribution() const
{
    TMap<ECharacterArchetype, int32> Distribution;
    
    for (const auto& ArchetypePair : CharactersByArchetype)
    {
        int32 ValidCount = 0;
        for (const TWeakObjectPtr<ACharacter>& WeakChar : ArchetypePair.Value)
        {
            if (WeakChar.IsValid())
            {
                ValidCount++;
            }
        }
        
        if (ValidCount > 0)
        {
            Distribution.Add(ArchetypePair.Key, ValidCount);
        }
    }
    
    return Distribution;
}

// Private helper methods implementation

void UCharacterArtistManager::SetupCharacterMeshComponents(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (!MeshComponent)
    {
        return;
    }
    
    // Set up MetaHuman component if not present
    UMetaHumanCharacterComponent* MetaHumanComp = Character->FindComponentByClass<UMetaHumanCharacterComponent>();
    if (!MetaHumanComp)
    {
        MetaHumanComp = NewObject<UMetaHumanCharacterComponent>(Character);
        Character->AddInstanceComponent(MetaHumanComp);
    }
    
    // Set up MetaHuman generator component
    UMetaHumanCharacterGenerator* Generator = Character->FindComponentByClass<UMetaHumanCharacterGenerator>();
    if (!Generator)
    {
        Generator = NewObject<UMetaHumanCharacterGenerator>(Character);
        Character->AddInstanceComponent(Generator);
    }
    
    // Generate character from archetype
    if (Generator)
    {
        Generator->GenerateCharacterFromArchetype(ArchetypeData, MeshComponent);
    }
}

void UCharacterArtistManager::ApplyCharacterCustomization(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    UMetaHumanCharacterGenerator* Generator = Character->FindComponentByClass<UMetaHumanCharacterGenerator>();
    if (Generator)
    {
        FMetaHumanVariationParams Params = Generator->GenerateVariationParams(ArchetypeData);
        Generator->ApplyVariationToMetaHuman(Character->GetMesh(), Params);
    }
}

void UCharacterArtistManager::SetupCharacterMaterials(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    // Apply material variations based on archetype
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (MeshComponent)
    {
        // This would apply specific material instances based on the archetype
        UE_LOG(LogTemp, Log, TEXT("Applied materials for archetype: %s"), *ArchetypeData.ArchetypeName);
    }
}

void UCharacterArtistManager::ApplyClothingAndAccessories(ACharacter* Character, const FCharacterArchetypeData& ArchetypeData)
{
    UMetaHumanCharacterGenerator* Generator = Character->FindComponentByClass<UMetaHumanCharacterGenerator>();
    if (Generator)
    {
        FCharacterClothingConfig ClothingConfig = Generator->GenerateClothingConfig(ArchetypeData);
        Generator->ApplyClothingConfig(Character->GetMesh(), ClothingConfig);
    }
}

bool UCharacterArtistManager::ShouldCreateDiverseVariant(const FCharacterArchetypeData& ArchetypeData)
{
    // Check if we need more diversity in current population
    float CurrentDiversityRatio = 0.0f;
    int32 TotalCharacters = GetTotalCharacterCount();
    
    if (TotalCharacters > 0)
    {
        // Calculate diversity based on age and build distribution
        int32 DominantAgeCount = 0;
        int32 DominantBuildCount = 0;
        
        for (const auto& AgePair : AgeGroupCount)
        {
            DominantAgeCount = FMath::Max(DominantAgeCount, AgePair.Value);
        }
        
        for (const auto& BuildPair : BuildTypeCount)
        {
            DominantBuildCount = FMath::Max(DominantBuildCount, BuildPair.Value);
        }
        
        CurrentDiversityRatio = 1.0f - ((float)(DominantAgeCount + DominantBuildCount) / (TotalCharacters * 2));
    }
    
    return CurrentDiversityRatio < DiversityTargetPercentage;
}

FCharacterArchetypeData UCharacterArtistManager::CreateDiverseVariant(const FCharacterArchetypeData& BaseArchetype)
{
    FCharacterArchetypeData DiverseVariant = BaseArchetype;
    
    // Randomize age group if needed
    if (FMath::RandBool())
    {
        TArray<ECharacterAgeGroup> AgeGroups = {
            ECharacterAgeGroup::Young,
            ECharacterAgeGroup::Adult,
            ECharacterAgeGroup::MiddleAged,
            ECharacterAgeGroup::Elder
        };
        DiverseVariant.AgeGroup = AgeGroups[FMath::RandRange(0, AgeGroups.Num() - 1)];
    }
    
    // Randomize build type if needed
    if (FMath::RandBool())
    {
        TArray<ECharacterBuild> BuildTypes = {
            ECharacterBuild::Slim,
            ECharacterBuild::Average,
            ECharacterBuild::Athletic,
            ECharacterBuild::Heavy
        };
        DiverseVariant.BuildType = BuildTypes[FMath::RandRange(0, BuildTypes.Num() - 1)];
    }
    
    // Add unique features
    DiverseVariant.UniqueFeatures += TEXT(" [Diverse Variant]");
    
    return DiverseVariant;
}

void UCharacterArtistManager::ApplyLifeExperience(ACharacter* Character, const FString& Experience)
{
    AddCharacterHistory(Character, Experience);
}

void UCharacterArtistManager::AddVisualDetails(ACharacter* Character, const TArray<FString>& Details)
{
    for (const FString& Detail : Details)
    {
        AddCharacterHistory(Character, Detail);
    }
}

void UCharacterArtistManager::ApplyWeatheringEffects(ACharacter* Character, float Intensity)
{
    if (!Character)
    {
        return;
    }
    
    // Apply visual weathering effects to materials and mesh
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (MeshComponent)
    {
        // This would modify material parameters to show wear and tear
        UE_LOG(LogTemp, Log, TEXT("Applied weathering effects with intensity: %.2f"), Intensity);
    }
}

void UCharacterArtistManager::UpdateCharacterLOD(ACharacter* Character, float DistanceToViewer)
{
    if (!Character)
    {
        return;
    }
    
    int32 LODLevel = 0;
    
    if (DistanceToViewer > LODDistanceThreshold * 3.0f)
    {
        LODLevel = 3; // Lowest quality
    }
    else if (DistanceToViewer > LODDistanceThreshold * 2.0f)
    {
        LODLevel = 2;
    }
    else if (DistanceToViewer > LODDistanceThreshold)
    {
        LODLevel = 1;
    }
    
    // Apply LOD level to character mesh
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (MeshComponent)
    {
        MeshComponent->SetForcedLOD(LODLevel + 1); // UE5 LOD is 1-based
    }
}

void UCharacterArtistManager::OptimizeCharacterForCrowd(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    // Apply crowd-specific optimizations
    SetCharacterQualityLevel(Character, 0); // Crowd quality
    UpdateCharacterLOD(Character, LODDistanceThreshold * 2.0f); // Force lower LOD
}

void UCharacterArtistManager::BalanceDiversityMetrics()
{
    // This would implement logic to balance character diversity across the population
    UE_LOG(LogTemp, Log, TEXT("Balanced diversity metrics"));
}