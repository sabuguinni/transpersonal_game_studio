#include "CharacterSystemManager.h"
#include "TranspersonalCharacterBase.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCharacterSystemManager::UCharacterSystemManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Set default protagonist name (will be overridden by Miguel's choice)
    ProtagonistName = TEXT("Dr. Samuel Carter");
}

void UCharacterSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCharacterSystem();
}

void UCharacterSystemManager::InitializeCharacterSystem()
{
    // Load character presets and initialize the system
    UE_LOG(LogTemp, Log, TEXT("Character System Manager: Initializing character system..."));
    
    // Initialize character archetypes
    CharacterPresets.Add(TEXT("Protagonist"), ProtagonistPreset);
    CharacterPresets.Add(TEXT("Survivor_Male_01"), nullptr); // To be loaded from MetaHuman assets
    CharacterPresets.Add(TEXT("Survivor_Female_01"), nullptr);
    CharacterPresets.Add(TEXT("Researcher_Male_01"), nullptr);
    CharacterPresets.Add(TEXT("Researcher_Female_01"), nullptr);
    
    UE_LOG(LogTemp, Log, TEXT("Character System Manager: System initialized with %d presets"), CharacterPresets.Num());
}

ATranspersonalCharacterBase* UCharacterSystemManager::CreateProtagonist(const FVector& SpawnLocation)
{
    if (!ProtagonistPreset.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Character System Manager: Protagonist preset not set!"));
        return nullptr;
    }
    
    // Spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    // Spawn the protagonist
    ATranspersonalCharacterBase* Protagonist = GetWorld()->SpawnActor<ATranspersonalCharacterBase>(
        ATranspersonalCharacterBase::StaticClass(),
        SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (Protagonist)
    {
        // Configure protagonist-specific properties
        Protagonist->SetCharacterName(ProtagonistName);
        Protagonist->SetCharacterArchetype(TEXT("Protagonist"));
        Protagonist->SetIsProtagonist(true);
        
        UE_LOG(LogTemp, Log, TEXT("Character System Manager: Protagonist '%s' created successfully"), *ProtagonistName);
    }
    
    return Protagonist;
}

ATranspersonalCharacterBase* UCharacterSystemManager::GenerateRandomNPC(const FVector& SpawnLocation, const FString& ArchetypeID)
{
    // Generate unique character variation
    FString CharacterHash = GenerateCharacterVariation(ArchetypeID);
    
    // Ensure uniqueness
    if (!IsCharacterUnique(CharacterHash))
    {
        UE_LOG(LogTemp, Warning, TEXT("Character System Manager: Failed to generate unique character for archetype %s"), *ArchetypeID);
        return nullptr;
    }
    
    // Spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    // Spawn the NPC
    ATranspersonalCharacterBase* NPC = GetWorld()->SpawnActor<ATranspersonalCharacterBase>(
        ATranspersonalCharacterBase::StaticClass(),
        SpawnLocation,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    if (NPC)
    {
        // Configure NPC properties
        NPC->SetCharacterArchetype(ArchetypeID);
        NPC->SetCharacterVariationHash(CharacterHash);
        NPC->SetIsProtagonist(false);
        
        // Add to tracking
        GeneratedCharacterHashes.Add(CharacterHash);
        
        UE_LOG(LogTemp, Log, TEXT("Character System Manager: NPC created with archetype %s"), *ArchetypeID);
    }
    
    return NPC;
}

FString UCharacterSystemManager::GenerateCharacterVariation(const FString& BaseArchetype)
{
    // Generate random variation parameters
    float FacialVariation = FMath::RandRange(-FacialVariationRange, FacialVariationRange);
    float BodyVariation = FMath::RandRange(-BodyVariationRange, BodyVariationRange);
    int32 SkinTone = FMath::RandRange(1, 10);
    int32 HairStyle = FMath::RandRange(1, 15);
    int32 EyeColor = FMath::RandRange(1, 8);
    
    // Create unique hash
    FString VariationHash = FString::Printf(TEXT("%s_F%.3f_B%.3f_S%d_H%d_E%d"),
        *BaseArchetype,
        FacialVariation,
        BodyVariation,
        SkinTone,
        HairStyle,
        EyeColor
    );
    
    return VariationHash;
}

bool UCharacterSystemManager::IsCharacterUnique(const FString& CharacterHash)
{
    // Check if we've reached the limit
    if (GeneratedCharacterHashes.Num() >= MaxUniqueCharacters)
    {
        return false;
    }
    
    // Check if this variation already exists
    return !GeneratedCharacterHashes.Contains(CharacterHash);
}