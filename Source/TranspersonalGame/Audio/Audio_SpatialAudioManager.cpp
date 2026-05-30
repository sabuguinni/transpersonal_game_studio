#include "Audio_SpatialAudioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/Engine.h"

AAudio_SpatialAudioManager::AAudio_SpatialAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    SpatialAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SpatialAudioComponent"));
    RootComponent = SpatialAudioComponent;
    
    EnvironmentalAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EnvironmentalAudioComponent"));
    EnvironmentalAudioComponent->SetupAttachment(RootComponent);

    // Initialize default values
    MasterSpatialVolume = 1.0f;
    MaxAudioDistance = 8000.0f;
    FootstepShakeIntensity = 1.5f;
    FootstepShakeRadius = 2000.0f;
    
    PlayerPawn = nullptr;
    LastAudioUpdateTime = 0.0f;
    AudioUpdateInterval = 0.1f; // Update 10 times per second
}

void AAudio_SpatialAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player pawn reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
    
    // Initialize dinosaur sound profiles
    InitializeDinosaurSoundProfiles();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: BeginPlay completed"));
}

void AAudio_SpatialAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastAudioUpdateTime += DeltaTime;
    if (LastAudioUpdateTime >= AudioUpdateInterval)
    {
        UpdateSpatialAudio(DeltaTime);
        LastAudioUpdateTime = 0.0f;
    }
}

void AAudio_SpatialAudioManager::PlaySoundAtLocation(USoundCue* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier)
{
    if (!Sound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: PlaySoundAtLocation - Sound is null"));
        return;
    }
    
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: PlaySoundAtLocation - PlayerPawn is null"));
        return;
    }
    
    // Calculate distance attenuation
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(Location, PlayerLocation);
    float Attenuation = CalculateDistanceAttenuation(Location, PlayerLocation, MaxAudioDistance);
    
    if (Attenuation > 0.01f) // Only play if audible
    {
        float FinalVolume = MasterSpatialVolume * VolumeMultiplier * Attenuation;
        
        UGameplayStatics::PlaySoundAtLocation(
            this,
            Sound,
            Location,
            FinalVolume,
            PitchMultiplier,
            0.0f, // Start time
            nullptr, // Attenuation override
            nullptr, // Concurrency settings
            nullptr  // Owner
        );
        
        UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Played sound at location (%f,%f,%f) with volume %f"), 
               Location.X, Location.Y, Location.Z, FinalVolume);
    }
}

void AAudio_SpatialAudioManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location)
{
    FAudio_DinosaurSoundProfile* Profile = FindDinosaurProfile(DinosaurType);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: No sound profile found for dinosaur type: %s"), *DinosaurType);
        return;
    }
    
    USoundCue* SoundToPlay = nullptr;
    
    if (SoundType == TEXT("Idle"))
    {
        SoundToPlay = Profile->IdleSound;
    }
    else if (SoundType == TEXT("Movement"))
    {
        SoundToPlay = Profile->MovementSound;
    }
    else if (SoundType == TEXT("Attack"))
    {
        SoundToPlay = Profile->AttackSound;
    }
    
    if (SoundToPlay)
    {
        PlaySoundAtLocation(SoundToPlay, Location, Profile->VolumeMultiplier);
        
        // Trigger screen shake for large dinosaur footsteps
        if (SoundType == TEXT("Movement") && (DinosaurType == TEXT("TRex") || DinosaurType == TEXT("Brachiosaurus")))
        {
            TriggerFootstepShake(Location, FootstepShakeIntensity);
        }
    }
}

void AAudio_SpatialAudioManager::RegisterDinosaurSoundProfile(const FAudio_DinosaurSoundProfile& Profile)
{
    // Check if profile already exists
    for (int32 i = 0; i < DinosaurSoundProfiles.Num(); i++)
    {
        if (DinosaurSoundProfiles[i].DinosaurType == Profile.DinosaurType)
        {
            DinosaurSoundProfiles[i] = Profile; // Update existing
            UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Updated sound profile for %s"), *Profile.DinosaurType);
            return;
        }
    }
    
    // Add new profile
    DinosaurSoundProfiles.Add(Profile);
    UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Added new sound profile for %s"), *Profile.DinosaurType);
}

void AAudio_SpatialAudioManager::SetMasterSpatialVolume(float Volume)
{
    MasterSpatialVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Master spatial volume set to %f"), MasterSpatialVolume);
}

float AAudio_SpatialAudioManager::CalculateDistanceAttenuation(FVector SoundLocation, FVector ListenerLocation, float MaxDistance)
{
    float Distance = FVector::Dist(SoundLocation, ListenerLocation);
    
    if (Distance >= MaxDistance)
    {
        return 0.0f;
    }
    
    // Linear attenuation with slight curve
    float NormalizedDistance = Distance / MaxDistance;
    float Attenuation = 1.0f - (NormalizedDistance * NormalizedDistance);
    
    return FMath::Clamp(Attenuation, 0.0f, 1.0f);
}

void AAudio_SpatialAudioManager::TriggerFootstepShake(FVector FootstepLocation, float Intensity)
{
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(FootstepLocation, PlayerLocation);
    
    if (Distance <= FootstepShakeRadius)
    {
        float ShakeAttenuation = 1.0f - (Distance / FootstepShakeRadius);
        float FinalIntensity = Intensity * ShakeAttenuation;
        
        if (APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController()))
        {
            // Create a simple camera shake effect
            // Note: In a full implementation, you would create a UCameraShakeBase subclass
            PC->ClientStartCameraShake(nullptr, FinalIntensity);
            
            UE_LOG(LogTemp, Log, TEXT("Audio_SpatialAudioManager: Triggered footstep shake with intensity %f at distance %f"), 
                   FinalIntensity, Distance);
        }
    }
}

void AAudio_SpatialAudioManager::PlayFootstepWithShake(FVector Location, const FString& DinosaurType)
{
    PlayDinosaurSound(DinosaurType, TEXT("Movement"), Location);
    
    // Additional shake for very large dinosaurs
    if (DinosaurType == TEXT("TRex") || DinosaurType == TEXT("Brachiosaurus"))
    {
        TriggerFootstepShake(Location, FootstepShakeIntensity * 1.5f);
    }
}

void AAudio_SpatialAudioManager::InitializeDinosaurSoundProfiles()
{
    // T-Rex profile
    FAudio_DinosaurSoundProfile TRexProfile;
    TRexProfile.DinosaurType = TEXT("TRex");
    TRexProfile.MaxHearingDistance = 8000.0f;
    TRexProfile.VolumeMultiplier = 1.5f;
    DinosaurSoundProfiles.Add(TRexProfile);
    
    // Velociraptor profile
    FAudio_DinosaurSoundProfile VelociraptorProfile;
    VelociraptorProfile.DinosaurType = TEXT("Velociraptor");
    VelociraptorProfile.MaxHearingDistance = 3000.0f;
    VelociraptorProfile.VolumeMultiplier = 0.8f;
    DinosaurSoundProfiles.Add(VelociraptorProfile);
    
    // Brachiosaurus profile
    FAudio_DinosaurSoundProfile BrachiosaurusProfile;
    BrachiosaurusProfile.DinosaurType = TEXT("Brachiosaurus");
    BrachiosaurusProfile.MaxHearingDistance = 10000.0f;
    BrachiosaurusProfile.VolumeMultiplier = 2.0f;
    DinosaurSoundProfiles.Add(BrachiosaurusProfile);
    
    // Triceratops profile
    FAudio_DinosaurSoundProfile TriceratopsProfile;
    TriceratopsProfile.DinosaurType = TEXT("Triceratops");
    TriceratopsProfile.MaxHearingDistance = 5000.0f;
    TriceratopsProfile.VolumeMultiplier = 1.2f;
    DinosaurSoundProfiles.Add(TriceratopsProfile);
    
    UE_LOG(LogTemp, Warning, TEXT("Audio_SpatialAudioManager: Initialized %d dinosaur sound profiles"), DinosaurSoundProfiles.Num());
}

void AAudio_SpatialAudioManager::UpdateSpatialAudio(float DeltaTime)
{
    // Update player pawn reference if needed
    if (!PlayerPawn && GetWorld())
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
    
    // Additional spatial audio updates can be added here
    // For example: updating environmental audio based on player location
}

FAudio_DinosaurSoundProfile* AAudio_SpatialAudioManager::FindDinosaurProfile(const FString& DinosaurType)
{
    for (FAudio_DinosaurSoundProfile& Profile : DinosaurSoundProfiles)
    {
        if (Profile.DinosaurType == DinosaurType)
        {
            return &Profile;
        }
    }
    return nullptr;
}