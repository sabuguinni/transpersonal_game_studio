#include "Audio_BiomeAudioManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

AAudio_BiomeAudioManager::AAudio_BiomeAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.7f);

    WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudio"));
    WindAudioComponent->SetupAttachment(RootComponent);
    WindAudioComponent->bAutoActivate = false;
    WindAudioComponent->SetVolumeMultiplier(0.5f);

    WildlifeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WildlifeAudio"));
    WildlifeAudioComponent->SetupAttachment(RootComponent);
    WildlifeAudioComponent->bAutoActivate = false;
    WildlifeAudioComponent->SetVolumeMultiplier(0.6f);

    // Initialize state
    CurrentBiome = EAudio_BiomeType::Savana;
    TargetBiome = EAudio_BiomeType::Savana;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    LastUpdateTime = 0.0f;
    PlayerPawn = nullptr;

    // Initialize biome audio database with default values
    BiomeAudioDatabase.SetNum(5);
    
    // Pantano (Swamp)
    BiomeAudioDatabase[0].BiomeType = EAudio_BiomeType::Pantano;
    BiomeAudioDatabase[0].BaseVolume = 0.8f;
    BiomeAudioDatabase[0].FadeInTime = 3.0f;
    BiomeAudioDatabase[0].FadeOutTime = 2.0f;

    // Floresta (Forest)
    BiomeAudioDatabase[1].BiomeType = EAudio_BiomeType::Floresta;
    BiomeAudioDatabase[1].BaseVolume = 0.7f;
    BiomeAudioDatabase[1].FadeInTime = 2.5f;
    BiomeAudioDatabase[1].FadeOutTime = 2.0f;

    // Savana (Savanna)
    BiomeAudioDatabase[2].BiomeType = EAudio_BiomeType::Savana;
    BiomeAudioDatabase[2].BaseVolume = 0.6f;
    BiomeAudioDatabase[2].FadeInTime = 2.0f;
    BiomeAudioDatabase[2].FadeOutTime = 1.5f;

    // Deserto (Desert)
    BiomeAudioDatabase[3].BiomeType = EAudio_BiomeType::Deserto;
    BiomeAudioDatabase[3].BaseVolume = 0.5f;
    BiomeAudioDatabase[3].FadeInTime = 2.0f;
    BiomeAudioDatabase[3].FadeOutTime = 1.5f;

    // Montanha (Mountain)
    BiomeAudioDatabase[4].BiomeType = EAudio_BiomeType::Montanha;
    BiomeAudioDatabase[4].BaseVolume = 0.4f;
    BiomeAudioDatabase[4].FadeInTime = 2.5f;
    BiomeAudioDatabase[4].FadeOutTime = 2.0f;
}

void AAudio_BiomeAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Find player pawn
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }

    // Start with current biome audio
    SetBiomeAudio(CurrentBiome);
}

void AAudio_BiomeAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!PlayerPawn)
    {
        return;
    }

    // Update biome detection periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        LastUpdateTime = CurrentTime;

        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        EAudio_BiomeType DetectedBiome = DetectCurrentBiome(PlayerLocation);

        if (DetectedBiome != CurrentBiome && !bIsTransitioning)
        {
            StartBiomeTransition(DetectedBiome);
        }
    }

    // Update transition if active
    if (bIsTransitioning)
    {
        UpdateAudioTransition(DeltaTime);
    }
}

EAudio_BiomeType AAudio_BiomeAudioManager::DetectCurrentBiome(const FVector& PlayerLocation)
{
    float X = PlayerLocation.X;
    float Y = PlayerLocation.Y;

    // Pantano (Swamp) - Southwest
    if (X >= -77500 && X <= -25000 && Y >= -76500 && Y <= -15000)
    {
        return EAudio_BiomeType::Pantano;
    }
    // Floresta (Forest) - Northwest  
    else if (X >= -77500 && X <= -15000 && Y >= 15000 && Y <= 76500)
    {
        return EAudio_BiomeType::Floresta;
    }
    // Deserto (Desert) - East
    else if (X >= 25000 && X <= 79500 && Y >= -30000 && Y <= 30000)
    {
        return EAudio_BiomeType::Deserto;
    }
    // Montanha (Mountain) - Northeast
    else if (X >= 15000 && X <= 79500 && Y >= 20000 && Y <= 76500)
    {
        return EAudio_BiomeType::Montanha;
    }
    // Savana (Savanna) - Center (default)
    else
    {
        return EAudio_BiomeType::Savana;
    }
}

void AAudio_BiomeAudioManager::StartBiomeTransition(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome)
    {
        return;
    }

    TargetBiome = NewBiome;
    bIsTransitioning = true;
    TransitionProgress = 0.0f;

    // Trigger blueprint event
    OnBiomeChanged(CurrentBiome, NewBiome);

    UE_LOG(LogTemp, Log, TEXT("BiomeAudioManager: Starting transition from %d to %d"), 
           (int32)CurrentBiome, (int32)NewBiome);
}

void AAudio_BiomeAudioManager::UpdateAudioTransition(float DeltaTime)
{
    FAudio_BiomeAudioData* CurrentData = GetBiomeAudioData(CurrentBiome);
    FAudio_BiomeAudioData* TargetData = GetBiomeAudioData(TargetBiome);

    if (!CurrentData || !TargetData)
    {
        bIsTransitioning = false;
        return;
    }

    float FadeOutTime = CurrentData->FadeOutTime;
    float FadeInTime = TargetData->FadeInTime;
    float TotalTransitionTime = FadeOutTime + FadeInTime;

    TransitionProgress += DeltaTime / TotalTransitionTime;

    if (TransitionProgress <= 0.5f)
    {
        // Fade out current biome
        float FadeOutProgress = TransitionProgress * 2.0f; // 0 to 1 over first half
        float VolumeMultiplier = 1.0f - FadeOutProgress;
        SetBiomeAudio(CurrentBiome, VolumeMultiplier);
    }
    else
    {
        // Fade in target biome
        float FadeInProgress = (TransitionProgress - 0.5f) * 2.0f; // 0 to 1 over second half
        SetBiomeAudio(TargetBiome, FadeInProgress);
    }

    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentBiome = TargetBiome;
        bIsTransitioning = false;
        TransitionProgress = 0.0f;
        SetBiomeAudio(CurrentBiome, 1.0f);

        UE_LOG(LogTemp, Log, TEXT("BiomeAudioManager: Transition to biome %d complete"), (int32)CurrentBiome);
    }
}

void AAudio_BiomeAudioManager::SetBiomeAudio(EAudio_BiomeType BiomeType, float VolumeMultiplier)
{
    FAudio_BiomeAudioData* BiomeData = GetBiomeAudioData(BiomeType);
    if (!BiomeData)
    {
        return;
    }

    float FinalVolume = BiomeData->BaseVolume * VolumeMultiplier;

    // Set ambient audio
    if (BiomeData->AmbientSound.IsValid())
    {
        if (USoundCue* SoundCue = BiomeData->AmbientSound.LoadSynchronous())
        {
            AmbientAudioComponent->SetSound(SoundCue);
            AmbientAudioComponent->SetVolumeMultiplier(FinalVolume);
            if (!AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->Play();
            }
        }
    }

    // Set wind audio
    if (BiomeData->WindSound.IsValid())
    {
        if (USoundCue* SoundCue = BiomeData->WindSound.LoadSynchronous())
        {
            WindAudioComponent->SetSound(SoundCue);
            WindAudioComponent->SetVolumeMultiplier(FinalVolume * 0.7f);
            if (!WindAudioComponent->IsPlaying())
            {
                WindAudioComponent->Play();
            }
        }
    }

    // Set wildlife audio
    if (BiomeData->WildlifeSound.IsValid())
    {
        if (USoundCue* SoundCue = BiomeData->WildlifeSound.LoadSynchronous())
        {
            WildlifeAudioComponent->SetSound(SoundCue);
            WildlifeAudioComponent->SetVolumeMultiplier(FinalVolume * 0.8f);
            if (!WildlifeAudioComponent->IsPlaying())
            {
                WildlifeAudioComponent->Play();
            }
        }
    }
}

FAudio_BiomeAudioData* AAudio_BiomeAudioManager::GetBiomeAudioData(EAudio_BiomeType BiomeType)
{
    for (FAudio_BiomeAudioData& Data : BiomeAudioDatabase)
    {
        if (Data.BiomeType == BiomeType)
        {
            return &Data;
        }
    }
    return nullptr;
}

void AAudio_BiomeAudioManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome != CurrentBiome)
    {
        StartBiomeTransition(NewBiome);
    }
}

void AAudio_BiomeAudioManager::SetMasterVolume(float Volume)
{
    Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->SetVolumeMultiplier(AmbientAudioComponent->VolumeMultiplier * Volume);
    }
    if (WindAudioComponent)
    {
        WindAudioComponent->SetVolumeMultiplier(WindAudioComponent->VolumeMultiplier * Volume);
    }
    if (WildlifeAudioComponent)
    {
        WildlifeAudioComponent->SetVolumeMultiplier(WildlifeAudioComponent->VolumeMultiplier * Volume);
    }
}