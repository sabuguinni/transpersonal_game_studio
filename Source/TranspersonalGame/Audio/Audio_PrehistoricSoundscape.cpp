#include "Audio_PrehistoricSoundscape.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AAudio_PrehistoricSoundscape::AAudio_PrehistoricSoundscape()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create ambience audio component
    AmbienceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbienceAudioComponent"));
    AmbienceAudioComponent->SetupAttachment(RootComponent);
    AmbienceAudioComponent->bAutoActivate = true;

    // Initialize default values
    CurrentBiome = EAudio_BiomeType::Forest;
    CurrentTimeOfDay = EAudio_TimeOfDay::Morning;
    MasterAmbienceVolume = 0.8f;
    TimeOfDayTransitionSpeed = 1.0f;
    DinosaurDetectionRadius = 3000.0f;
    CurrentVolumeMultiplier = 1.0f;
    bIsTransitioning = false;

    // Initialize biome ambience data with defaults
    BiomeAmbienceData.SetNum(5);
    
    // Forest biome
    BiomeAmbienceData[0].BiomeType = EAudio_BiomeType::Forest;
    BiomeAmbienceData[0].BaseVolume = 0.7f;
    BiomeAmbienceData[0].AttenuationRadius = 2500.0f;
    
    // Plains biome  
    BiomeAmbienceData[1].BiomeType = EAudio_BiomeType::Plains;
    BiomeAmbienceData[1].BaseVolume = 0.6f;
    BiomeAmbienceData[1].AttenuationRadius = 4000.0f;
    
    // River biome
    BiomeAmbienceData[2].BiomeType = EAudio_BiomeType::River;
    BiomeAmbienceData[2].BaseVolume = 0.8f;
    BiomeAmbienceData[2].AttenuationRadius = 1500.0f;
    
    // Mountain biome
    BiomeAmbienceData[3].BiomeType = EAudio_BiomeType::Mountain;
    BiomeAmbienceData[3].BaseVolume = 0.5f;
    BiomeAmbienceData[3].AttenuationRadius = 3000.0f;
    
    // Cave biome
    BiomeAmbienceData[4].BiomeType = EAudio_BiomeType::Cave;
    BiomeAmbienceData[4].BaseVolume = 0.4f;
    BiomeAmbienceData[4].AttenuationRadius = 800.0f;
}

void AAudio_PrehistoricSoundscape::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the ambience system
    UpdateCurrentAmbience();
    
    UE_LOG(LogTemp, Log, TEXT("Prehistoric Soundscape initialized in %s biome"), 
           *UEnum::GetValueAsString(CurrentBiome));
}

void AAudio_PrehistoricSoundscape::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Check for nearby dinosaurs every few frames
    static float DinosaurCheckTimer = 0.0f;
    DinosaurCheckTimer += DeltaTime;
    if (DinosaurCheckTimer >= 0.5f)  // Check twice per second
    {
        CheckDinosaurProximity();
        DinosaurCheckTimer = 0.0f;
    }
    
    // Handle time of day transitions
    if (bIsTransitioning)
    {
        // Smooth volume transitions during time changes
        CurrentVolumeMultiplier = FMath::FInterpTo(CurrentVolumeMultiplier, 1.0f, 
                                                  DeltaTime, TimeOfDayTransitionSpeed);
        
        if (FMath::IsNearlyEqual(CurrentVolumeMultiplier, 1.0f, 0.01f))
        {
            bIsTransitioning = false;
        }
        
        UpdateAmbienceVolume(MasterAmbienceVolume * CurrentVolumeMultiplier);
    }
}

void AAudio_PrehistoricSoundscape::SetBiomeType(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UpdateCurrentAmbience();
        
        UE_LOG(LogTemp, Log, TEXT("Biome changed to: %s"), 
               *UEnum::GetValueAsString(CurrentBiome));
    }
}

void AAudio_PrehistoricSoundscape::SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        bIsTransitioning = true;
        CurrentVolumeMultiplier = 0.3f;  // Fade out briefly during transition
        
        UpdateCurrentAmbience();
        
        UE_LOG(LogTemp, Log, TEXT("Time of day changed to: %s"), 
               *UEnum::GetValueAsString(CurrentTimeOfDay));
    }
}

void AAudio_PrehistoricSoundscape::PlayDinosaurProximityAlert(const FString& DinosaurType, float Distance)
{
    if (!AmbienceAudioComponent)
        return;
        
    USoundCue* AlertSound = nullptr;
    
    if (DinosaurType.Contains(TEXT("TRex")) || DinosaurType.Contains(TEXT("Rex")))
    {
        AlertSound = TRexApproachSound.LoadSynchronous();
    }
    else if (DinosaurType.Contains(TEXT("Raptor")))
    {
        AlertSound = RaptorPackSound.LoadSynchronous();
    }
    
    if (AlertSound)
    {
        // Calculate volume based on distance
        float AlertVolume = FMath::Clamp(1.0f - (Distance / DinosaurDetectionRadius), 0.1f, 1.0f);
        
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), AlertSound, GetActorLocation(), 
                                            AlertVolume, 1.0f, 0.0f);
        
        UE_LOG(LogTemp, Warning, TEXT("DINOSAUR PROXIMITY ALERT: %s at distance %.0f"), 
               *DinosaurType, Distance);
    }
}

void AAudio_PrehistoricSoundscape::UpdateAmbienceVolume(float NewVolume)
{
    MasterAmbienceVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    if (AmbienceAudioComponent)
    {
        AmbienceAudioComponent->SetVolumeMultiplier(MasterAmbienceVolume);
    }
}

void AAudio_PrehistoricSoundscape::UpdateCurrentAmbience()
{
    FAudio_BiomeAmbience* BiomeData = GetCurrentBiomeData();
    if (!BiomeData || !AmbienceAudioComponent)
        return;
    
    // Select appropriate sound based on time of day
    USoundCue* SelectedSound = nullptr;
    if (CurrentTimeOfDay == EAudio_TimeOfDay::Night)
    {
        SelectedSound = BiomeData->NightAmbientSound.LoadSynchronous();
    }
    else
    {
        SelectedSound = BiomeData->DayAmbientSound.LoadSynchronous();
    }
    
    if (SelectedSound)
    {
        AmbienceAudioComponent->SetSound(SelectedSound);
        AmbienceAudioComponent->SetVolumeMultiplier(BiomeData->BaseVolume * MasterAmbienceVolume);
        
        if (!AmbienceAudioComponent->IsPlaying())
        {
            AmbienceAudioComponent->Play();
        }
    }
}

void AAudio_PrehistoricSoundscape::CheckDinosaurProximity()
{
    if (!GetWorld())
        return;
        
    // Get player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector MyLocation = GetActorLocation();
    
    // Find all actors in range
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsInRadiusOfClass(GetWorld(), MyLocation, 
                                                 DinosaurDetectionRadius, AActor::StaticClass(), 
                                                 NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this)
            continue;
            
        FString ActorName = Actor->GetName();
        
        // Check if this is a dinosaur actor
        if (ActorName.Contains(TEXT("TRex")) || ActorName.Contains(TEXT("Raptor")) || 
            ActorName.Contains(TEXT("Dinosaur")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            // Only alert if dinosaur is getting close to player
            if (Distance < DinosaurDetectionRadius * 0.5f)  // Alert at half detection radius
            {
                PlayDinosaurProximityAlert(ActorName, Distance);
            }
        }
    }
}

FAudio_BiomeAmbience* AAudio_PrehistoricSoundscape::GetCurrentBiomeData()
{
    for (FAudio_BiomeAmbience& BiomeData : BiomeAmbienceData)
    {
        if (BiomeData.BiomeType == CurrentBiome)
        {
            return &BiomeData;
        }
    }
    
    // Return first biome as fallback
    return BiomeAmbienceData.Num() > 0 ? &BiomeAmbienceData[0] : nullptr;
}