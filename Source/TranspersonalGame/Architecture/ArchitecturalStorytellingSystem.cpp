// Copyright Transpersonal Game Studio. All Rights Reserved.
// Architectural Storytelling System Implementation
// Agent #07 - Architecture & Interior Agent

#include "ArchitecturalStorytellingSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchitecturalStorytelling, Log, All);

AArchitecturalStorytellingSystem::AArchitecturalStorytellingSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update every second
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create main structure mesh
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);
    MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructureMesh->SetCollisionResponseToAllChannels(ECR_Block);
    
    // Create PCG component for interior generation
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    
    // Create instanced mesh component for props
    PropInstancedMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PropInstancedMesh"));
    PropInstancedMesh->SetupAttachment(RootComponent);
    
    // Create ambient audio component
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->bAutoActivate = false;
    
    // Initialize default values
    StructureType = EArchitectureType::BasicShelter;
    InhabitationStory = EInhabitationStory::PeacefulFamily;
    DecayStage = EDecayStage::Fresh;
    StorytellingIntensity = 0.7f;
    bHasActiveNarrative = true;
    bIsPlayerDiscovered = false;
    
    // Initialize interior zones
    InitializeDefaultInteriorZones();
}

void AArchitecturalStorytellingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Initializing Architectural Storytelling System"));
    
    // Generate the structure and its story
    GenerateStructureStory();
    PopulateInteriorWithProps();
    ApplyDecayEffects();
    SetupAmbientEffects();
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Architectural storytelling system initialized successfully"));
}

void AArchitecturalStorytellingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update atmospheric effects based on player proximity
    UpdateAtmosphericEffects();
    
    // Check for player discovery
    CheckPlayerDiscovery();
}

void AArchitecturalStorytellingSystem::GenerateStructureStory()
{
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Generating structure story for type: %s"), 
           *UEnum::GetValueAsString(StructureType));
    
    // Clear existing story elements
    StorytellingProps.Empty();
    InteriorZones.Empty();
    
    // Generate story based on structure type and inhabitation story
    switch (InhabitationStory)
    {
        case EInhabitationStory::PeacefulFamily:
            GeneratePeacefulFamilyStory();
            break;
        case EInhabitationStory::LoneHunter:
            GenerateLoneHunterStory();
            break;
        case EInhabitationStory::TradingPost:
            GenerateTradingPostStory();
            break;
        case EInhabitationStory::DefensiveOutpost:
            GenerateDefensiveOutpostStory();
            break;
        case EInhabitationStory::HastyFlight:
            GenerateHastyFlightStory();
            break;
        case EInhabitationStory::TragedyStruck:
            GenerateTragedyStruckStory();
            break;
        default:
            GenerateGenericStory();
            break;
    }
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Generated %d story props for structure"), 
           StorytellingProps.Num());
}

void AArchitecturalStorytellingSystem::PopulateInteriorWithProps()
{
    if (!PropInstancedMesh)
    {
        UE_LOG(LogArchitecturalStorytelling, Warning, TEXT("PropInstancedMesh is null, cannot populate interior"));
        return;
    }
    
    // Clear existing instances
    PropInstancedMesh->ClearInstances();
    
    // Place props in each interior zone
    for (const FInteriorZoneConfig& Zone : InteriorZones)
    {
        PlacePropsInZone(Zone);
    }
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Populated interior with %d prop instances"), 
           PropInstancedMesh->GetInstanceCount());
}

void AArchitecturalStorytellingSystem::ApplyDecayEffects()
{
    if (!MainStructureMesh)
    {
        return;
    }
    
    // Create dynamic material for decay effects
    UMaterialInterface* BaseMaterial = MainStructureMesh->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DecayMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        
        // Apply decay parameters based on decay stage
        float DecayAmount = GetDecayAmountForStage(DecayStage);
        float VegetationOvergrowth = GetVegetationOvergrowthForStage(DecayStage);
        float WeatheringIntensity = GetWeatheringIntensityForStage(DecayStage);
        
        DecayMaterial->SetScalarParameterValue(TEXT("DecayAmount"), DecayAmount);
        DecayMaterial->SetScalarParameterValue(TEXT("VegetationOvergrowth"), VegetationOvergrowth);
        DecayMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), WeatheringIntensity);
        
        MainStructureMesh->SetMaterial(0, DecayMaterial);
        
        UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Applied decay effects: Decay=%.2f, Vegetation=%.2f, Weathering=%.2f"), 
               DecayAmount, VegetationOvergrowth, WeatheringIntensity);
    }
}

void AArchitecturalStorytellingSystem::SetupAmbientEffects()
{
    // Setup ambient audio based on story and decay
    if (AmbientAudio && GetAmbientSoundForStory())
    {
        AmbientAudio->SetSound(GetAmbientSoundForStory());
        AmbientAudio->SetVolumeMultiplier(GetAmbientVolumeForDecay());
        AmbientAudio->Play();
        
        UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Setup ambient audio for story"));
    }
    
    // Setup particle effects for atmosphere
    SetupAtmosphericParticles();
}

void AArchitecturalStorytellingSystem::UpdateAtmosphericEffects()
{
    // Get player distance for atmospheric intensity
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    float AtmosphericIntensity = FMath::Clamp(1.0f - (DistanceToPlayer / 2000.0f), 0.0f, 1.0f);
    
    // Update ambient audio volume
    if (AmbientAudio)
    {
        float BaseVolume = GetAmbientVolumeForDecay();
        AmbientAudio->SetVolumeMultiplier(BaseVolume * AtmosphericIntensity);
    }
    
    // Update particle intensity
    UpdateParticleIntensity(AtmosphericIntensity);
}

void AArchitecturalStorytellingSystem::CheckPlayerDiscovery()
{
    if (bIsPlayerDiscovered)
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistanceToPlayer <= DiscoveryRadius)
    {
        OnPlayerDiscovered();
    }
}

void AArchitecturalStorytellingSystem::OnPlayerDiscovered()
{
    bIsPlayerDiscovered = true;
    
    UE_LOG(LogArchitecturalStorytelling, Log, TEXT("Player discovered structure with story: %s"), 
           *UEnum::GetValueAsString(InhabitationStory));
    
    // Trigger discovery events
    OnStructureDiscovered.Broadcast(this, InhabitationStory);
    
    // Blueprint implementable event
    K2_OnPlayerDiscovered();
}

void AArchitecturalStorytellingSystem::GeneratePeacefulFamilyStory()
{
    // Create family-oriented props
    FStorytellingProp SleepingArea;
    SleepingArea.PropName = TEXT("Family Sleeping Area");
    SleepingArea.EvidenceType = EHabitationEvidence::SleepingArea;
    SleepingArea.PreferredZone = EInteriorZone::Sleeping;
    SleepingArea.StoryDescription = TEXT("Multiple sleeping mats arranged together, showing a family unit");
    SleepingArea.StoryImportance = 8;
    StorytellingProps.Add(SleepingArea);
    
    FStorytellingProp ChildrenToys;
    ChildrenToys.PropName = TEXT("Children's Toys");
    ChildrenToys.EvidenceType = EHabitationEvidence::ChildrenToys;
    ChildrenToys.PreferredZone = EInteriorZone::Living;
    ChildrenToys.StoryDescription = TEXT("Small carved wooden animals and simple toys");
    ChildrenToys.StoryImportance = 6;
    StorytellingProps.Add(ChildrenToys);
    
    FStorytellingProp CookingHearth;
    CookingHearth.PropName = TEXT("Family Cooking Hearth");
    CookingHearth.EvidenceType = EHabitationEvidence::CookingHearth;
    CookingHearth.PreferredZone = EInteriorZone::Cooking;
    CookingHearth.StoryDescription = TEXT("Large hearth with evidence of regular family meals");
    CookingHearth.StoryImportance = 7;
    StorytellingProps.Add(CookingHearth);
}

void AArchitecturalStorytellingSystem::GenerateLoneHunterStory()
{
    // Create hunter-oriented props
    FStorytellingProp WeaponRack;
    WeaponRack.PropName = TEXT("Hunter's Weapon Rack");
    WeaponRack.EvidenceType = EHabitationEvidence::WeaponRack;
    WeaponRack.PreferredZone = EInteriorZone::Work;
    WeaponRack.StoryDescription = TEXT("Well-maintained spears and hunting tools");
    WeaponRack.StoryImportance = 9;
    StorytellingProps.Add(WeaponRack);
    
    FStorytellingProp TrophyCollection;
    TrophyCollection.PropName = TEXT("Trophy Collection");
    TrophyCollection.EvidenceType = EHabitationEvidence::ComfortItems;
    TrophyCollection.PreferredZone = EInteriorZone::Living;
    TrophyCollection.StoryDescription = TEXT("Dinosaur teeth and claws displayed as trophies");
    TrophyCollection.StoryImportance = 7;
    StorytellingProps.Add(TrophyCollection);
}

void AArchitecturalStorytellingSystem::GenerateHastyFlightStory()
{
    // Create abandonment evidence
    FStorytellingProp AbandonedItems;
    AbandonedItems.PropName = TEXT("Hastily Abandoned Belongings");
    AbandonedItems.EvidenceType = EHabitationEvidence::AbandonedItems;
    AbandonedItems.PreferredZone = EInteriorZone::Living;
    AbandonedItems.StoryDescription = TEXT("Personal items scattered as if dropped in panic");
    AbandonedItems.StoryImportance = 9;
    StorytellingProps.Add(AbandonedItems);
    
    FStorytellingProp BarricadeMarks;
    BarricadeMarks.PropName = TEXT("Barricade Remains");
    BarricadeMarks.EvidenceType = EHabitationEvidence::BarricadeMarks;
    BarricadeMarks.PreferredZone = EInteriorZone::Entrance;
    BarricadeMarks.StoryDescription = TEXT("Evidence of desperate attempt to block the entrance");
    BarricadeMarks.StoryImportance = 8;
    StorytellingProps.Add(BarricadeMarks);
}

void AArchitecturalStorytellingSystem::GenerateGenericStory()
{
    // Create basic habitation evidence
    FStorytellingProp BasicSleeping;
    BasicSleeping.PropName = TEXT("Basic Sleeping Area");
    BasicSleeping.EvidenceType = EHabitationEvidence::SleepingArea;
    BasicSleeping.PreferredZone = EInteriorZone::Sleeping;
    BasicSleeping.StoryDescription = TEXT("Simple sleeping arrangement");
    BasicSleeping.StoryImportance = 5;
    StorytellingProps.Add(BasicSleeping);
    
    FStorytellingProp BasicCooking;
    BasicCooking.PropName = TEXT("Basic Fire Pit");
    BasicCooking.EvidenceType = EHabitationEvidence::CookingHearth;
    BasicCooking.PreferredZone = EInteriorZone::Cooking;
    BasicCooking.StoryDescription = TEXT("Simple cooking fire");
    BasicCooking.StoryImportance = 5;
    StorytellingProps.Add(BasicCooking);
}

void AArchitecturalStorytellingSystem::PlacePropsInZone(const FInteriorZoneConfig& Zone)
{
    // Find props that belong to this zone
    TArray<FStorytellingProp> ZoneProps;
    for (const FStorytellingProp& Prop : StorytellingProps)
    {
        if (Prop.PreferredZone == Zone.ZoneType)
        {
            ZoneProps.Add(Prop);
        }
    }
    
    // Place props within zone bounds
    for (const FStorytellingProp& Prop : ZoneProps)
    {
        FVector PropLocation = Zone.ZoneCenter + FMath::VRand() * Zone.ZoneRadius * 0.5f;
        FRotator PropRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        FVector PropScale = FVector(FMath::RandRange(Prop.ScaleRange.X, Prop.ScaleRange.Y));
        
        FTransform PropTransform(PropRotation, PropLocation, PropScale);
        PropInstancedMesh->AddInstance(PropTransform);
    }
}

float AArchitecturalStorytellingSystem::GetDecayAmountForStage(EDecayStage Stage) const
{
    switch (Stage)
    {
        case EDecayStage::Fresh: return 0.0f;
        case EDecayStage::Early: return 0.2f;
        case EDecayStage::Moderate: return 0.5f;
        case EDecayStage::Advanced: return 0.8f;
        case EDecayStage::Ruins: return 0.95f;
        case EDecayStage::Archaeological: return 1.0f;
        default: return 0.0f;
    }
}

float AArchitecturalStorytellingSystem::GetVegetationOvergrowthForStage(EDecayStage Stage) const
{
    switch (Stage)
    {
        case EDecayStage::Fresh: return 0.0f;
        case EDecayStage::Early: return 0.1f;
        case EDecayStage::Moderate: return 0.3f;
        case EDecayStage::Advanced: return 0.6f;
        case EDecayStage::Ruins: return 0.9f;
        case EDecayStage::Archaeological: return 1.0f;
        default: return 0.0f;
    }
}

float AArchitecturalStorytellingSystem::GetWeatheringIntensityForStage(EDecayStage Stage) const
{
    switch (Stage)
    {
        case EDecayStage::Fresh: return 0.0f;
        case EDecayStage::Early: return 0.3f;
        case EDecayStage::Moderate: return 0.6f;
        case EDecayStage::Advanced: return 0.8f;
        case EDecayStage::Ruins: return 0.95f;
        case EDecayStage::Archaeological: return 1.0f;
        default: return 0.0f;
    }
}

USoundCue* AArchitecturalStorytellingSystem::GetAmbientSoundForStory() const
{
    // Return appropriate ambient sound based on story and decay
    // This would be implemented with actual sound assets
    return nullptr;
}

float AArchitecturalStorytellingSystem::GetAmbientVolumeForDecay() const
{
    // Fresher structures have more ambient activity sounds
    switch (DecayStage)
    {
        case EDecayStage::Fresh: return 0.8f;
        case EDecayStage::Early: return 0.6f;
        case EDecayStage::Moderate: return 0.4f;
        case EDecayStage::Advanced: return 0.2f;
        case EDecayStage::Ruins: return 0.1f;
        case EDecayStage::Archaeological: return 0.05f;
        default: return 0.0f;
    }
}

void AArchitecturalStorytellingSystem::SetupAtmosphericParticles()
{
    // Setup particle systems for dust, smoke remnants, etc.
    // Implementation would create and configure particle components
}

void AArchitecturalStorytellingSystem::UpdateParticleIntensity(float Intensity)
{
    // Update particle system intensity based on player proximity
    // Implementation would modify particle parameters
}

void AArchitecturalStorytellingSystem::InitializeDefaultInteriorZones()
{
    // Create default interior zone layout
    FInteriorZoneConfig LivingZone;
    LivingZone.ZoneType = EInteriorZone::Living;
    LivingZone.ZoneName = TEXT("Living Area");
    LivingZone.ZoneCenter = FVector(0, 0, 0);
    LivingZone.ZoneRadius = 200.0f;
    InteriorZones.Add(LivingZone);
    
    FInteriorZoneConfig SleepingZone;
    SleepingZone.ZoneType = EInteriorZone::Sleeping;
    SleepingZone.ZoneName = TEXT("Sleeping Area");
    SleepingZone.ZoneCenter = FVector(150, 0, 0);
    SleepingZone.ZoneRadius = 150.0f;
    InteriorZones.Add(SleepingZone);
    
    FInteriorZoneConfig CookingZone;
    CookingZone.ZoneType = EInteriorZone::Cooking;
    CookingZone.ZoneName = TEXT("Cooking Area");
    CookingZone.ZoneCenter = FVector(-150, 0, 0);
    CookingZone.ZoneRadius = 100.0f;
    InteriorZones.Add(CookingZone);
}

// Additional story generation methods
void AArchitecturalStorytellingSystem::GenerateTradingPostStory()
{
    FStorytellingProp TradingGoods;
    TradingGoods.PropName = TEXT("Trading Goods Cache");
    TradingGoods.EvidenceType = EHabitationEvidence::StorageCache;
    TradingGoods.PreferredZone = EInteriorZone::Storage;
    TradingGoods.StoryDescription = TEXT("Organized storage of various trade goods");
    TradingGoods.StoryImportance = 8;
    StorytellingProps.Add(TradingGoods);
}

void AArchitecturalStorytellingSystem::GenerateDefensiveOutpostStory()
{
    FStorytellingProp WeaponCache;
    WeaponCache.PropName = TEXT("Weapon Cache");
    WeaponCache.EvidenceType = EHabitationEvidence::WeaponRack;
    WeaponCache.PreferredZone = EInteriorZone::Work;
    WeaponCache.StoryDescription = TEXT("Military-style weapon storage");
    WeaponCache.StoryImportance = 9;
    StorytellingProps.Add(WeaponCache);
}

void AArchitecturalStorytellingSystem::GenerateTragedyStruckStory()
{
    FStorytellingProp Memorial;
    Memorial.PropName = TEXT("Memorial Corner");
    Memorial.EvidenceType = EHabitationEvidence::MemorialCorner;
    Memorial.PreferredZone = EInteriorZone::Living;
    Memorial.StoryDescription = TEXT("A corner dedicated to remembering the lost");
    Memorial.StoryImportance = 10;
    StorytellingProps.Add(Memorial);
}