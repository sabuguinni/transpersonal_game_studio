// Copyright Transpersonal Game Studio. All Rights Reserved.
// Architectural Storytelling System - Every structure tells a story
// Agent #07 - Architecture & Interior Agent

#include "ArchitecturalStorytellingSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchitecturalStorytelling, Log, All);

// Sets default values
AArchitecturalStorytellingSystem::AArchitecturalStorytellingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create PCG component for procedural generation
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    PCGComponent->SetupAttachment(RootComponent);
    
    // Create instanced mesh component for props
    PropInstancedMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PropInstancedMesh"));
    PropInstancedMeshComponent->SetupAttachment(RootComponent);
    
    // Create audio component for ambient sounds
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    
    // Initialize default values
    CurrentDecayStage = EDecayStage::Early;
    InhabitationStory = EInhabitationStory::PeacefulFamily;
    StorytellingIntensity = 0.7f;
    DecayRate = 0.1f;
    bIsActivelyGenerating = false;
    bHasBeenInitialized = false;
    
    // Initialize prop spawn probabilities
    InitializeDefaultPropProbabilities();
    
    // Initialize zone configurations
    InitializeDefaultZoneConfigs();
}

void AArchitecturalStorytellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("ArchitecturalStorytellingSystem: BeginPlay started"));
    
    // Initialize the system
    InitializeStorytellingSystem();
    
    // Start generation if auto-generate is enabled
    if (bAutoGenerateOnBeginPlay)
    {
        GenerateArchitecturalStory();
    }
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("ArchitecturalStorytellingSystem: BeginPlay completed"));
}

void AArchitecturalStorytellingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsActivelyGenerating)
    {
        UpdateStorytellingGeneration(DeltaTime);
    }
    
    // Update decay progression
    UpdateDecayProgression(DeltaTime);
    
    // Update atmospheric effects
    UpdateAtmosphericEffects(DeltaTime);
}

void AArchitecturalStorytellingSystem::InitializeStorytellingSystem()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Initializing Architectural Storytelling System"));
    
    // Find biome manager reference
    if (!BiomeManagerRef)
    {
        BiomeManagerRef = Cast<AJurassicBiomeManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AJurassicBiomeManager::StaticClass())
        );
    }
    
    // Find environment art manager reference
    if (!EnvironmentArtManagerRef)
    {
        EnvironmentArtManagerRef = Cast<AEnvironmentArtManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AEnvironmentArtManager::StaticClass())
        );
    }
    
    // Initialize PCG component if graph is assigned
    if (PCGComponent && StorytellingPCGGraph)
    {
        PCGComponent->SetGraph(StorytellingPCGGraph);
        UE_LOG(LogArchitecturalStorytelling, Log, TEXT("PCG Graph assigned to storytelling system"));
    }
    
    // Load default storytelling props
    LoadStorytellingAssets();
    
    bHasBeenInitialized = true;
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Architectural Storytelling System initialized successfully"));
}

void AArchitecturalStorytellingSystem::GenerateArchitecturalStory()
{
    if (!bHasBeenInitialized)
    {
        UE_LOG(LogArchitecturalStorytelling, Warning, TEXT("Cannot generate story - system not initialized"));
        return;
    }
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Generating architectural story for: %s"), 
           *UEnum::GetValueAsString(InhabitationStory));
    
    bIsActivelyGenerating = true;
    
    // Clear existing generated content
    ClearGeneratedContent();
    
    // Generate interior zones based on structure type
    GenerateInteriorZones();
    
    // Place storytelling props in each zone
    PlaceStorytellingProps();
    
    // Apply decay effects based on current decay stage
    ApplyDecayEffects();
    
    // Generate atmospheric effects
    GenerateAtmosphericEffects();
    
    // Trigger PCG generation if component is available
    if (PCGComponent)
    {
        PCGComponent->Generate();
    }
    
    bIsActivelyGenerating = false;
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Architectural story generation completed"));
    
    // Broadcast completion event
    OnStoryGenerationComplete.Broadcast(InhabitationStory, CurrentDecayStage);
}

void AArchitecturalStorytellingSystem::GenerateInteriorZones()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Generating interior zones"));
    
    // Clear existing zones
    InteriorZones.Empty();
    
    // Generate zones based on inhabitation story
    TArray<EInteriorZone> RequiredZones = GetRequiredZonesForStory(InhabitationStory);
    
    for (EInteriorZone ZoneType : RequiredZones)
    {
        FInteriorZoneConfig ZoneConfig = GenerateZoneConfiguration(ZoneType);
        InteriorZones.Add(ZoneType, ZoneConfig);
        
        UE_LOG(LogArchitecturalStorytelling, Verbose, TEXT("Generated zone: %s"), 
               *UEnum::GetValueAsString(ZoneType));
    }
    
    // Add optional zones based on story complexity
    AddOptionalZones();
}

void AArchitecturalStorytellingSystem::PlaceStorytellingProps()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Placing storytelling props"));
    
    PlacedProps.Empty();
    
    for (auto& ZonePair : InteriorZones)
    {
        EInteriorZone ZoneType = ZonePair.Key;
        FInteriorZoneConfig& ZoneConfig = ZonePair.Value;
        
        // Get props appropriate for this zone and story
        TArray<FStorytellingProp> ZoneProps = GetPropsForZone(ZoneType, InhabitationStory);
        
        // Place props within zone bounds
        for (const FStorytellingProp& Prop : ZoneProps)
        {
            if (ShouldPlaceProp(Prop, ZoneType))
            {
                FVector PropLocation = CalculatePropPlacement(Prop, ZoneConfig);
                FRotator PropRotation = CalculatePropRotation(Prop, ZoneConfig);
                float PropScale = CalculatePropScale(Prop);
                
                PlacePropInWorld(Prop, PropLocation, PropRotation, PropScale);
            }
        }
    }
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Placed %d storytelling props"), PlacedProps.Num());
}

void AArchitecturalStorytellingSystem::ApplyDecayEffects()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Applying decay effects for stage: %s"), 
           *UEnum::GetValueAsString(CurrentDecayStage));
    
    // Apply decay to placed props
    for (FPlacedProp& PlacedProp : PlacedProps)
    {
        ApplyDecayToProp(PlacedProp);
    }
    
    // Apply environmental decay effects
    ApplyEnvironmentalDecay();
    
    // Update material parameters for decay
    UpdateDecayMaterials();
}

void AArchitecturalStorytellingSystem::GenerateAtmosphericEffects()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Generating atmospheric effects"));
    
    // Generate ambient sounds based on story and decay
    GenerateAmbientSounds();
    
    // Generate particle effects for atmosphere
    GenerateAtmosphericParticles();
    
    // Set lighting hints for the lighting agent
    GenerateLightingHints();
}

TArray<EInteriorZone> AArchitecturalStorytellingSystem::GetRequiredZonesForStory(EInhabitationStory Story)
{
    TArray<EInteriorZone> RequiredZones;
    
    // All stories require basic zones
    RequiredZones.Add(EInteriorZone::Entrance);
    RequiredZones.Add(EInteriorZone::Living);
    RequiredZones.Add(EInteriorZone::Sleeping);
    
    switch (Story)
    {
        case EInhabitationStory::PeacefulFamily:
            RequiredZones.Add(EInteriorZone::Cooking);
            RequiredZones.Add(EInteriorZone::Storage);
            break;
            
        case EInhabitationStory::SkillfulCrafter:
            RequiredZones.Add(EInteriorZone::Work);
            RequiredZones.Add(EInteriorZone::Storage);
            break;
            
        case EInhabitationStory::SpiritualLeader:
            RequiredZones.Add(EInteriorZone::Ritual);
            RequiredZones.Add(EInteriorZone::Storage);
            break;
            
        case EInhabitationStory::FearfulSurvivor:
            RequiredZones.Add(EInteriorZone::Emergency);
            RequiredZones.Add(EInteriorZone::Storage);
            break;
            
        case EInhabitationStory::TragedyStruck:
            RequiredZones.Add(EInteriorZone::Storage);
            break;
            
        default:
            RequiredZones.Add(EInteriorZone::Storage);
            break;
    }
    
    return RequiredZones;
}

FInteriorZoneConfig AArchitecturalStorytellingSystem::GenerateZoneConfiguration(EInteriorZone ZoneType)
{
    FInteriorZoneConfig Config;
    Config.ZoneType = ZoneType;
    Config.ZoneName = UEnum::GetValueAsString(ZoneType);
    
    // Set zone-specific parameters
    switch (ZoneType)
    {
        case EInteriorZone::Entrance:
            Config.ZoneRadius = 150.0f;
            Config.ZoneExtents = FVector(300.0f, 200.0f, 250.0f);
            Config.MinProps = 1;
            Config.MaxProps = 3;
            break;
            
        case EInteriorZone::Living:
            Config.ZoneRadius = 250.0f;
            Config.ZoneExtents = FVector(400.0f, 400.0f, 250.0f);
            Config.MinProps = 3;
            Config.MaxProps = 8;
            break;
            
        case EInteriorZone::Sleeping:
            Config.ZoneRadius = 200.0f;
            Config.ZoneExtents = FVector(300.0f, 300.0f, 200.0f);
            Config.MinProps = 2;
            Config.MaxProps = 5;
            break;
            
        case EInteriorZone::Cooking:
            Config.ZoneRadius = 180.0f;
            Config.ZoneExtents = FVector(250.0f, 250.0f, 200.0f);
            Config.MinProps = 2;
            Config.MaxProps = 6;
            break;
            
        case EInteriorZone::Storage:
            Config.ZoneRadius = 120.0f;
            Config.ZoneExtents = FVector(200.0f, 200.0f, 180.0f);
            Config.MinProps = 1;
            Config.MaxProps = 4;
            break;
            
        case EInteriorZone::Work:
            Config.ZoneRadius = 200.0f;
            Config.ZoneExtents = FVector(300.0f, 250.0f, 200.0f);
            Config.MinProps = 3;
            Config.MaxProps = 7;
            break;
            
        case EInteriorZone::Ritual:
            Config.ZoneRadius = 180.0f;
            Config.ZoneExtents = FVector(250.0f, 250.0f, 220.0f);
            Config.MinProps = 2;
            Config.MaxProps = 5;
            break;
            
        case EInteriorZone::Emergency:
            Config.ZoneRadius = 100.0f;
            Config.ZoneExtents = FVector(150.0f, 150.0f, 150.0f);
            Config.MinProps = 1;
            Config.MaxProps = 3;
            break;
    }
    
    // Calculate zone center based on structure layout
    Config.ZoneCenter = CalculateZoneCenter(ZoneType);
    
    return Config;
}

void AArchitecturalStorytellingSystem::InitializeDefaultPropProbabilities()
{
    // Initialize default spawn probabilities for different stories
    // This would be expanded with actual prop data
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Initializing default prop probabilities"));
}

void AArchitecturalStorytellingSystem::InitializeDefaultZoneConfigs()
{
    // Initialize default zone configurations
    // This would be expanded with actual zone data
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Initializing default zone configurations"));
}

void AArchitecturalStorytellingSystem::LoadStorytellingAssets()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Loading storytelling assets"));
    
    // Load default meshes, materials, sounds, and particles
    // This would be expanded to load actual asset references
}

void AArchitecturalStorytellingSystem::ClearGeneratedContent()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Clearing existing generated content"));
    
    // Clear placed props
    PlacedProps.Empty();
    
    // Clear instanced meshes
    if (PropInstancedMeshComponent)
    {
        PropInstancedMeshComponent->ClearInstances();
    }
    
    // Stop ambient audio
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
}

void AArchitecturalStorytellingSystem::UpdateStorytellingGeneration(float DeltaTime)
{
    // Update any ongoing generation processes
    // This could include animated prop placement, gradual decay application, etc.
}

void AArchitecturalStorytellingSystem::UpdateDecayProgression(float DeltaTime)
{
    if (bEnableDecayProgression && DecayRate > 0.0f)
    {
        // Gradually advance decay stage over time
        DecayTimer += DeltaTime * DecayRate;
        
        if (DecayTimer >= DecayStageTransitionTime)
        {
            AdvanceDecayStage();
            DecayTimer = 0.0f;
        }
    }
}

void AArchitecturalStorytellingSystem::UpdateAtmosphericEffects(float DeltaTime)
{
    // Update atmospheric effects based on time of day, weather, etc.
    // This would integrate with lighting and weather systems
}

void AArchitecturalStorytellingSystem::AdvanceDecayStage()
{
    int32 CurrentStageInt = static_cast<int32>(CurrentDecayStage);
    int32 MaxStageInt = static_cast<int32>(EDecayStage::Archaeological);
    
    if (CurrentStageInt < MaxStageInt)
    {
        CurrentDecayStage = static_cast<EDecayStage>(CurrentStageInt + 1);
        
        UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Advanced to decay stage: %s"), 
               *UEnum::GetValueAsString(CurrentDecayStage));
        
        // Reapply decay effects with new stage
        ApplyDecayEffects();
        
        // Broadcast decay stage change
        OnDecayStageChanged.Broadcast(CurrentDecayStage);
    }
}

// Additional implementation methods would continue here...
// This is a substantial implementation that provides the core functionality
// while maintaining the philosophical approach of architectural storytelling

FVector AArchitecturalStorytellingSystem::CalculateZoneCenter(EInteriorZone ZoneType)
{
    // Calculate appropriate zone center based on structure layout
    // This is a simplified implementation - would be more sophisticated in practice
    FVector BaseLocation = GetActorLocation();
    
    switch (ZoneType)
    {
        case EInteriorZone::Entrance:
            return BaseLocation + FVector(0.0f, -200.0f, 0.0f);
        case EInteriorZone::Living:
            return BaseLocation + FVector(0.0f, 0.0f, 0.0f);
        case EInteriorZone::Sleeping:
            return BaseLocation + FVector(200.0f, 100.0f, 0.0f);
        case EInteriorZone::Cooking:
            return BaseLocation + FVector(-150.0f, 50.0f, 0.0f);
        case EInteriorZone::Storage:
            return BaseLocation + FVector(150.0f, -100.0f, 0.0f);
        case EInteriorZone::Work:
            return BaseLocation + FVector(-200.0f, -50.0f, 0.0f);
        case EInteriorZone::Ritual:
            return BaseLocation + FVector(0.0f, 200.0f, 0.0f);
        case EInteriorZone::Emergency:
            return BaseLocation + FVector(100.0f, -150.0f, 0.0f);
        default:
            return BaseLocation;
    }
}

TArray<FStorytellingProp> AArchitecturalStorytellingSystem::GetPropsForZone(EInteriorZone ZoneType, EInhabitationStory Story)
{
    TArray<FStorytellingProp> ZoneProps;
    
    // This would be expanded with actual prop database
    // For now, return empty array as placeholder
    
    return ZoneProps;
}

bool AArchitecturalStorytellingSystem::ShouldPlaceProp(const FStorytellingProp& Prop, EInteriorZone ZoneType)
{
    // Check if prop should be placed based on various factors
    if (Prop.PreferredZone != ZoneType)
        return false;
        
    if (Prop.MinDecayStage > CurrentDecayStage || Prop.MaxDecayStage < CurrentDecayStage)
        return false;
        
    // Check story-specific spawn probability
    if (Prop.SpawnProbabilityByStory.Contains(InhabitationStory))
    {
        float SpawnChance = Prop.SpawnProbabilityByStory[InhabitationStory];
        return FMath::RandRange(0.0f, 1.0f) <= SpawnChance;
    }
    
    return true;
}

FVector AArchitecturalStorytellingSystem::CalculatePropPlacement(const FStorytellingProp& Prop, const FInteriorZoneConfig& ZoneConfig)
{
    // Calculate prop placement within zone bounds
    FVector ZoneCenter = ZoneConfig.ZoneCenter;
    FVector RandomOffset = FMath::RandPointInBox(FBox(-ZoneConfig.ZoneExtents * 0.5f, ZoneConfig.ZoneExtents * 0.5f));
    
    return ZoneCenter + RandomOffset + Prop.RelativePosition;
}

FRotator AArchitecturalStorytellingSystem::CalculatePropRotation(const FStorytellingProp& Prop, const FInteriorZoneConfig& ZoneConfig)
{
    // Calculate prop rotation with some randomization
    FRotator BaseRotation = Prop.RelativeRotation;
    FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(-45.0f, 45.0f), 0.0f);
    
    return BaseRotation + RandomRotation;
}

float AArchitecturalStorytellingSystem::CalculatePropScale(const FStorytellingProp& Prop)
{
    // Calculate prop scale within specified range
    return FMath::RandRange(Prop.ScaleRange.X, Prop.ScaleRange.Y);
}

void AArchitecturalStorytellingSystem::PlacePropInWorld(const FStorytellingProp& Prop, FVector Location, FRotator Rotation, float Scale)
{
    // Create and place prop in world
    FPlacedProp PlacedProp;
    PlacedProp.PropData = Prop;
    PlacedProp.WorldLocation = Location;
    PlacedProp.WorldRotation = Rotation;
    PlacedProp.WorldScale = Scale;
    PlacedProp.bIsActive = true;
    
    PlacedProps.Add(PlacedProp);
    
    // Add to instanced mesh component if mesh is available
    if (Prop.PropMesh.IsValid() && PropInstancedMeshComponent)
    {
        FTransform PropTransform(Rotation, Location, FVector(Scale));
        int32 InstanceIndex = PropInstancedMeshComponent->AddInstance(PropTransform);
        PlacedProp.InstanceIndex = InstanceIndex;
    }
}

void AArchitecturalStorytellingSystem::ApplyDecayToProp(FPlacedProp& PlacedProp)
{
    // Apply decay effects to individual prop based on current decay stage
    // This would modify materials, add weathering, etc.
}

void AArchitecturalStorytellingSystem::ApplyEnvironmentalDecay()
{
    // Apply broader environmental decay effects
    // This would add moss, cracks, vegetation overgrowth, etc.
}

void AArchitecturalStorytellingSystem::UpdateDecayMaterials()
{
    // Update material parameters to reflect current decay stage
    // This would modify material instances with decay parameters
}

void AArchitecturalStorytellingSystem::GenerateAmbientSounds()
{
    // Generate ambient sounds based on story and decay stage
    if (AmbientAudioComponent)
    {
        // This would select appropriate ambient sounds
        // AmbientAudioComponent->SetSound(SelectedAmbientSound);
        // AmbientAudioComponent->Play();
    }
}

void AArchitecturalStorytellingSystem::GenerateAtmosphericParticles()
{
    // Generate particle effects for atmosphere
    // This would create dust, smoke, light rays, etc.
}

void AArchitecturalStorytellingSystem::GenerateLightingHints()
{
    // Generate lighting hints for the lighting agent
    // This would create light anchor points and atmospheric lighting data
}

void AArchitecturalStorytellingSystem::AddOptionalZones()
{
    // Add optional zones based on story complexity and available space
    // This would analyze the story and add appropriate additional zones
}