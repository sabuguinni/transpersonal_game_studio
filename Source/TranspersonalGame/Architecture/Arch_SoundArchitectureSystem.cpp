#include "Arch_SoundArchitectureSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "TranspersonalGame/SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogSoundArchitecture, Log, All);

UArch_SoundArchitectureSystem::UArch_SoundArchitectureSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds
    
    // Initialize acoustic properties
    BaseReverbMultiplier = 1.0f;
    EchoDecayRate = 0.8f;
    SoundOcclusionFactor = 0.3f;
    MaxAcousticRange = 5000.0f;
    
    // Initialize structure types
    InitializeAcousticStructures();
}

void UArch_SoundArchitectureSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogSoundArchitecture, Log, TEXT("Sound Architecture System initialized"));
    
    // Find and register all architectural structures in the world
    RegisterWorldStructures();
    
    // Setup acoustic zones
    SetupAcousticZones();
}

void UArch_SoundArchitectureSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update acoustic properties based on player position
    UpdateAcousticEnvironment();
}

void UArch_SoundArchitectureSystem::InitializeAcousticStructures()
{
    // Cave systems - high reverb, echo
    FArch_AcousticProperties CaveProps;
    CaveProps.ReverbMultiplier = 2.5f;
    CaveProps.EchoStrength = 0.8f;
    CaveProps.SoundAbsorption = 0.2f;
    CaveProps.FrequencyDamping = 0.6f;
    CaveProps.AmbientSoundType = EArch_AmbientSoundType::CaveDrops;
    AcousticStructureTypes.Add(EArch_StructureType::Cave, CaveProps);
    
    // Stone arches - moderate reverb, focused acoustics
    FArch_AcousticProperties ArchProps;
    ArchProps.ReverbMultiplier = 1.8f;
    ArchProps.EchoStrength = 0.5f;
    ArchProps.SoundAbsorption = 0.4f;
    ArchProps.FrequencyDamping = 0.3f;
    ArchProps.AmbientSoundType = EArch_AmbientSoundType::WindThroughStone;
    AcousticStructureTypes.Add(EArch_StructureType::StoneArch, ArchProps);
    
    // Cliff faces - strong echo, wind effects
    FArch_AcousticProperties CliffProps;
    CliffProps.ReverbMultiplier = 2.0f;
    CliffProps.EchoStrength = 0.9f;
    CliffProps.SoundAbsorption = 0.1f;
    CliffProps.FrequencyDamping = 0.2f;
    CliffProps.AmbientSoundType = EArch_AmbientSoundType::WindThroughStone;
    AcousticStructureTypes.Add(EArch_StructureType::CliffFace, CliffProps);
    
    // Primitive shelters - muffled sound, organic absorption
    FArch_AcousticProperties ShelterProps;
    ShelterProps.ReverbMultiplier = 0.8f;
    ShelterProps.EchoStrength = 0.2f;
    ShelterProps.SoundAbsorption = 0.7f;
    ShelterProps.FrequencyDamping = 0.8f;
    ShelterProps.AmbientSoundType = EArch_AmbientSoundType::CreakingWood;
    AcousticStructureTypes.Add(EArch_StructureType::PrimitiveShelter, ShelterProps);
}

void UArch_SoundArchitectureSystem::RegisterWorldStructures()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find all static mesh actors that could be architectural structures
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            FString ActorName = Actor->GetName().ToLower();
            
            // Classify structure type based on name/properties
            EArch_StructureType StructureType = ClassifyStructureType(ActorName);
            
            if (StructureType != EArch_StructureType::None)
            {
                RegisterAcousticStructure(Actor, StructureType);
            }
        }
    }
    
    UE_LOG(LogSoundArchitecture, Log, TEXT("Registered %d acoustic structures"), AcousticStructures.Num());
}

EArch_StructureType UArch_SoundArchitectureSystem::ClassifyStructureType(const FString& ActorName)
{
    if (ActorName.Contains("cave") || ActorName.Contains("cavern"))
    {
        return EArch_StructureType::Cave;
    }
    else if (ActorName.Contains("arch") || ActorName.Contains("outcrop"))
    {
        return EArch_StructureType::StoneArch;
    }
    else if (ActorName.Contains("cliff") || ActorName.Contains("rock"))
    {
        return EArch_StructureType::CliffFace;
    }
    else if (ActorName.Contains("shelter") || ActorName.Contains("dwelling"))
    {
        return EArch_StructureType::PrimitiveShelter;
    }
    
    return EArch_StructureType::None;
}

void UArch_SoundArchitectureSystem::RegisterAcousticStructure(AActor* Structure, EArch_StructureType StructureType)
{
    if (!Structure) return;
    
    FArch_AcousticStructure AcousticStructure;
    AcousticStructure.StructureActor = Structure;
    AcousticStructure.StructureType = StructureType;
    AcousticStructure.Location = Structure->GetActorLocation();
    
    // Get acoustic properties for this structure type
    if (AcousticStructureTypes.Contains(StructureType))
    {
        AcousticStructure.AcousticProperties = AcousticStructureTypes[StructureType];
    }
    
    // Calculate influence radius based on structure size
    FVector Origin, BoxExtent;
    Structure->GetActorBounds(false, Origin, BoxExtent);
    AcousticStructure.InfluenceRadius = FMath::Max(BoxExtent.Size(), 1000.0f);
    
    AcousticStructures.Add(AcousticStructure);
    
    // Create audio component for ambient sounds
    CreateAmbientAudioComponent(Structure, AcousticStructure.AcousticProperties);
}

void UArch_SoundArchitectureSystem::CreateAmbientAudioComponent(AActor* Structure, const FArch_AcousticProperties& Properties)
{
    if (!Structure) return;
    
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(Structure);
    if (AudioComp)
    {
        AudioComp->AttachToComponent(Structure->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        
        // Set ambient sound based on structure type
        // Note: Sound assets would need to be loaded from Content Browser
        AudioComp->bAutoActivate = true;
        AudioComp->VolumeMultiplier = 0.3f;
        AudioComp->PitchMultiplier = 1.0f;
        
        Structure->AddInstanceComponent(AudioComp);
        AudioComp->RegisterComponent();
    }
}

void UArch_SoundArchitectureSystem::SetupAcousticZones()
{
    // Create acoustic zones around major structures
    for (const FArch_AcousticStructure& Structure : AcousticStructures)
    {
        FArch_AcousticZone Zone;
        Zone.CenterLocation = Structure.Location;
        Zone.Radius = Structure.InfluenceRadius;
        Zone.AcousticProperties = Structure.AcousticProperties;
        Zone.Priority = GetZonePriority(Structure.StructureType);
        
        AcousticZones.Add(Zone);
    }
    
    UE_LOG(LogSoundArchitecture, Log, TEXT("Created %d acoustic zones"), AcousticZones.Num());
}

int32 UArch_SoundArchitectureSystem::GetZonePriority(EArch_StructureType StructureType)
{
    switch (StructureType)
    {
        case EArch_StructureType::Cave: return 100;
        case EArch_StructureType::PrimitiveShelter: return 90;
        case EArch_StructureType::StoneArch: return 70;
        case EArch_StructureType::CliffFace: return 50;
        default: return 10;
    }
}

void UArch_SoundArchitectureSystem::UpdateAcousticEnvironment()
{
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Find the highest priority acoustic zone affecting the player
    FArch_AcousticProperties CurrentProperties;
    int32 HighestPriority = -1;
    
    for (const FArch_AcousticZone& Zone : AcousticZones)
    {
        float Distance = FVector::Dist(PlayerLocation, Zone.CenterLocation);
        if (Distance <= Zone.Radius && Zone.Priority > HighestPriority)
        {
            CurrentProperties = Zone.AcousticProperties;
            HighestPriority = Zone.Priority;
        }
    }
    
    // Apply acoustic properties to audio system
    if (HighestPriority >= 0)
    {
        ApplyAcousticProperties(CurrentProperties);
    }
}

void UArch_SoundArchitectureSystem::ApplyAcousticProperties(const FArch_AcousticProperties& Properties)
{
    // This would interface with UE5's audio system to apply reverb, echo, etc.
    // For now, we'll log the changes
    UE_LOG(LogSoundArchitecture, Verbose, TEXT("Applying acoustic properties: Reverb=%.2f, Echo=%.2f"), 
        Properties.ReverbMultiplier, Properties.EchoStrength);
    
    // Store current properties for other systems to query
    CurrentAcousticProperties = Properties;
}

FArch_AcousticProperties UArch_SoundArchitectureSystem::GetCurrentAcousticProperties() const
{
    return CurrentAcousticProperties;
}

TArray<FArch_AcousticStructure> UArch_SoundArchitectureSystem::GetNearbyAcousticStructures(const FVector& Location, float Radius) const
{
    TArray<FArch_AcousticStructure> NearbyStructures;
    
    for (const FArch_AcousticStructure& Structure : AcousticStructures)
    {
        float Distance = FVector::Dist(Location, Structure.Location);
        if (Distance <= Radius)
        {
            NearbyStructures.Add(Structure);
        }
    }
    
    return NearbyStructures;
}