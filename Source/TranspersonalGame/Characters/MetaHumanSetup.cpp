// MetaHuman Setup and Integration
// Transpersonal Game Studio — Character Artist Agent #09
// Production Cycle: PROD_JURASSIC_001

#include "MetaHumanSetup.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetaHumanSetup, Log, All);

AMetaHumanCharacter::AMetaHumanCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize MetaHuman mesh component
    MetaHumanMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanMesh"));
    RootComponent = MetaHumanMesh;
    
    // Set default animation blueprint
    static ConstructorHelpers::FClassFinder<UAnimBlueprint> AnimBPClass(
        TEXT("/Game/MetaHumans/Common/Common/ABP_MetaHuman_PostProcess")
    );
    if (AnimBPClass.Class)
    {
        MetaHumanMesh->SetAnimInstanceClass(AnimBPClass.Class);
    }
    
    // Initialize character archetype
    CharacterArchetype = ECharacterArchetype::Protagonist;
    SurvivalLevel = 1.0f;
    EnvironmentalDamage = 0.0f;
    
    // Setup LOD configuration
    SetupLODConfiguration();
}

void AMetaHumanCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply archetype-specific settings
    ApplyArchetypeSettings();
    
    // Initialize survival state
    InitializeSurvivalState();
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("MetaHuman character initialized: %s"), 
           *GetName());
}

void AMetaHumanCharacter::SetupLODConfiguration()
{
    if (!MetaHumanMesh) return;
    
    // Configure LOD settings for performance
    MetaHumanMesh->SetForcedLOD(0); // Start with highest detail
    
    // Setup automatic LOD distances
    TArray<float> LODDistances = {0.0f, 500.0f, 1500.0f, 3000.0f};
    
    for (int32 i = 0; i < LODDistances.Num(); ++i)
    {
        if (MetaHumanMesh->GetSkeletalMeshAsset())
        {
            // LOD configuration will be handled by Nanite if enabled
            UE_LOG(LogMetaHumanSetup, Log, TEXT("LOD %d configured at distance: %f"), 
                   i, LODDistances[i]);
        }
    }
}

void AMetaHumanCharacter::ApplyArchetypeSettings()
{
    switch (CharacterArchetype)
    {
        case ECharacterArchetype::Protagonist:
            ApplyProtagonistSettings();
            break;
            
        case ECharacterArchetype::TribalNative:
            ApplyTribalSettings();
            break;
            
        case ECharacterArchetype::TimeTraveler:
            ApplyTimeTravelerSettings();
            break;
            
        case ECharacterArchetype::BackgroundNPC:
            ApplyBackgroundNPCSettings();
            break;
            
        default:
            UE_LOG(LogMetaHumanSetup, Warning, 
                   TEXT("Unknown character archetype for %s"), *GetName());
            break;
    }
}

void AMetaHumanCharacter::ApplyProtagonistSettings()
{
    // Protagonist gets highest quality settings
    if (MetaHumanMesh)
    {
        MetaHumanMesh->SetForcedLOD(-1); // Use automatic LOD
        
        // Load protagonist-specific materials
        LoadMaterialSet(TEXT("Protagonist"));
        
        // Enable advanced features
        MetaHumanMesh->SetCastShadow(true);
        MetaHumanMesh->SetReceivesDecals(true);
    }
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("Applied protagonist settings"));
}

void AMetaHumanCharacter::ApplyTribalSettings()
{
    // Tribal characters have unique markings and clothing
    LoadMaterialSet(TEXT("Tribal"));
    
    // Apply tribal-specific modifications
    ApplyTribalMarkings();
    SetTribalClothing();
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("Applied tribal settings"));
}

void AMetaHumanCharacter::ApplyTimeTravelerSettings()
{
    // Time travelers show modern-to-primitive transition
    LoadMaterialSet(TEXT("TimeTraveler"));
    
    // Apply damage and weathering based on survival time
    ApplyEnvironmentalDamage(EnvironmentalDamage);
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("Applied time traveler settings"));
}

void AMetaHumanCharacter::ApplyBackgroundNPCSettings()
{
    // Background NPCs use optimized settings
    if (MetaHumanMesh)
    {
        MetaHumanMesh->SetForcedLOD(1); // Start with medium LOD
        MetaHumanMesh->SetCastShadow(false); // Optimize shadows
    }
    
    LoadMaterialSet(TEXT("Background"));
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("Applied background NPC settings"));
}

void AMetaHumanCharacter::LoadMaterialSet(const FString& MaterialSetName)
{
    // Load material set based on archetype
    FString MaterialPath = FString::Printf(
        TEXT("/Game/Characters/Materials/%s/"), *MaterialSetName
    );
    
    // This would load the appropriate material instances
    // Implementation depends on specific material organization
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("Loading material set: %s"), 
           *MaterialSetName);
}

void AMetaHumanCharacter::ApplyTribalMarkings()
{
    // Apply procedural tribal markings
    // This would use material parameters to add tribal tattoos/paint
    
    if (MetaHumanMesh && MetaHumanMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = 
            MetaHumanMesh->CreateAndSetMaterialInstanceDynamic(0);
            
        if (DynamicMaterial)
        {
            // Set tribal marking parameters
            DynamicMaterial->SetScalarParameterValue(TEXT("TribalMarkings"), 1.0f);
            DynamicMaterial->SetVectorParameterValue(TEXT("TribalColor"), 
                FLinearColor(0.8f, 0.2f, 0.1f, 1.0f));
        }
    }
}

void AMetaHumanCharacter::SetTribalClothing()
{
    // Set tribal-appropriate clothing materials
    // This would swap out modern clothing for primitive alternatives
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("Applied tribal clothing"));
}

void AMetaHumanCharacter::ApplyEnvironmentalDamage(float DamageAmount)
{
    // Apply weathering and damage based on survival time
    if (MetaHumanMesh && MetaHumanMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = 
            MetaHumanMesh->CreateAndSetMaterialInstanceDynamic(0);
            
        if (DynamicMaterial)
        {
            DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), DamageAmount);
            DynamicMaterial->SetScalarParameterValue(TEXT("WearLevel"), DamageAmount * 0.8f);
            DynamicMaterial->SetScalarParameterValue(TEXT("SunDamage"), DamageAmount * 0.6f);
        }
    }
    
    UE_LOG(LogMetaHumanSetup, Log, TEXT("Applied environmental damage: %f"), 
           DamageAmount);
}

void AMetaHumanCharacter::InitializeSurvivalState()
{
    // Initialize survival-related visual states
    UpdateSurvivalAppearance();
}

void AMetaHumanCharacter::UpdateSurvivalAppearance()
{
    // Update character appearance based on survival level
    float InverseSurvival = 1.0f - SurvivalLevel;
    
    // Apply fatigue, dirt, and wear based on survival state
    ApplyEnvironmentalDamage(InverseSurvival * 0.7f);
    
    // Update facial expressions and posture
    UpdateFacialState(SurvivalLevel);
}

void AMetaHumanCharacter::UpdateFacialState(float WellbeingLevel)
{
    // Update facial animation parameters based on character state
    if (MetaHumanMesh && MetaHumanMesh->GetAnimInstance())
    {
        // This would interface with the facial animation system
        // to show stress, fatigue, fear, etc.
        
        UE_LOG(LogMetaHumanSetup, Log, TEXT("Updated facial state: %f"), 
               WellbeingLevel);
    }
}

void AMetaHumanCharacter::SetCharacterArchetype(ECharacterArchetype NewArchetype)
{
    if (CharacterArchetype != NewArchetype)
    {
        CharacterArchetype = NewArchetype;
        ApplyArchetypeSettings();
        
        UE_LOG(LogMetaHumanSetup, Log, TEXT("Character archetype changed to: %d"), 
               (int32)NewArchetype);
    }
}

void AMetaHumanCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update character state over time
    UpdateCharacterState(DeltaTime);
}

void AMetaHumanCharacter::UpdateCharacterState(float DeltaTime)
{
    // Gradually update character appearance based on environmental factors
    // This could include dirt accumulation, clothing wear, etc.
    
    static float UpdateTimer = 0.0f;
    UpdateTimer += DeltaTime;
    
    // Update every 5 seconds to avoid performance issues
    if (UpdateTimer >= 5.0f)
    {
        UpdateTimer = 0.0f;
        
        // Subtle changes over time
        if (SurvivalLevel > 0.1f)
        {
            SurvivalLevel = FMath::Max(0.1f, SurvivalLevel - 0.001f);
            UpdateSurvivalAppearance();
        }
    }
}