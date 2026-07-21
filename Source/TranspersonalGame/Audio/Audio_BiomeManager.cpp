#include "Audio_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

AAudio_BiomeManager::AAudio_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio components
    PrimaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryAudioComponent"));
    PrimaryAudioComponent->SetupAttachment(RootComponent);
    PrimaryAudioComponent->bAutoActivate = false;

    SecondaryAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryAudioComponent"));
    SecondaryAudioComponent->SetupAttachment(RootComponent);
    SecondaryAudioComponent->bAutoActivate = false;

    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudioComponent"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;

    // Initialize default values
    CurrentBiome = EBiomeType::Savanna;
    MasterVolume = 1.0f;
    FadeTransitionTime = 3.0f;
    bAutoDetectBiome = true;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
    TransitionDuration = 3.0f;
    TransitionTargetBiome = EBiomeType::Savanna;

    SetupBiomeCenters();
}

void AAudio_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAudioData();
    UpdateAudioComponents();
}

void AAudio_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle biome transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
        
        if (TransitionProgress >= 1.0f)
        {
            // Complete transition
            SetCurrentBiome(TransitionTargetBiome);
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }

    // Auto-detect biome based on player location
    if (bAutoDetectBiome && !bIsTransitioning)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            EBiomeType DetectedBiome = DetectBiomeFromLocation(PlayerLocation);
            
            if (DetectedBiome != CurrentBiome)
            {
                TransitionToBiome(DetectedBiome, FadeTransitionTime);
            }
        }
    }
}

void AAudio_BiomeManager::SetCurrentBiome(EBiomeType NewBiome)
{
    if (CurrentBiome != NewBiome)
    {
        CurrentBiome = NewBiome;
        UpdateAudioComponents();
    }
}

void AAudio_BiomeManager::TransitionToBiome(EBiomeType TargetBiome, float TransitionDuration)
{
    if (TargetBiome != CurrentBiome && !bIsTransitioning)
    {
        bIsTransitioning = true;
        TransitionTargetBiome = TargetBiome;
        this->TransitionDuration = TransitionDuration;
        TransitionTimer = 0.0f;

        // Start fading out current audio
        if (PrimaryAudioComponent && PrimaryAudioComponent->IsPlaying())
        {
            FadeAudioComponent(PrimaryAudioComponent, 0.0f, TransitionDuration * 0.5f);
        }
        if (SecondaryAudioComponent && SecondaryAudioComponent->IsPlaying())
        {
            FadeAudioComponent(SecondaryAudioComponent, 0.0f, TransitionDuration * 0.5f);
        }
    }
}

void AAudio_BiomeManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UpdateAudioComponents();
}

void AAudio_BiomeManager::SetWeatherAudio(USoundCue* WeatherSound, float Volume)
{
    if (WeatherAudioComponent)
    {
        if (WeatherSound)
        {
            WeatherAudioComponent->SetSound(WeatherSound);
            WeatherAudioComponent->SetVolumeMultiplier(Volume * MasterVolume);
            WeatherAudioComponent->Play();
        }
        else
        {
            WeatherAudioComponent->Stop();
        }
    }
}

void AAudio_BiomeManager::StopAllAudio()
{
    if (PrimaryAudioComponent) PrimaryAudioComponent->Stop();
    if (SecondaryAudioComponent) SecondaryAudioComponent->Stop();
    if (WeatherAudioComponent) WeatherAudioComponent->Stop();
}

EBiomeType AAudio_BiomeManager::DetectBiomeFromLocation(FVector WorldLocation)
{
    EBiomeType ClosestBiome = EBiomeType::Savanna;
    float ClosestDistance = FLT_MAX;

    for (const auto& BiomePair : BiomeCenters)
    {
        float Distance = CalculateDistanceToBiomeCenter(BiomePair.Key, WorldLocation);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = BiomePair.Key;
        }
    }

    return ClosestBiome;
}

bool AAudio_BiomeManager::IsAudioPlaying() const
{
    return (PrimaryAudioComponent && PrimaryAudioComponent->IsPlaying()) ||
           (SecondaryAudioComponent && SecondaryAudioComponent->IsPlaying()) ||
           (WeatherAudioComponent && WeatherAudioComponent->IsPlaying());
}

void AAudio_BiomeManager::InitializeBiomeAudioData()
{
    // Initialize default audio data for each biome
    FAudio_BiomeAudioData SavannaData;
    SavannaData.AmbientVolume = 0.6f;
    SavannaData.AudioRange = 20000.0f;
    BiomeAudioMap.Add(EBiomeType::Savanna, SavannaData);

    FAudio_BiomeAudioData SwampData;
    SwampData.AmbientVolume = 0.7f;
    SwampData.AudioRange = 15000.0f;
    BiomeAudioMap.Add(EBiomeType::Swamp, SwampData);

    FAudio_BiomeAudioData ForestData;
    ForestData.AmbientVolume = 0.8f;
    ForestData.AudioRange = 12000.0f;
    BiomeAudioMap.Add(EBiomeType::Forest, ForestData);

    FAudio_BiomeAudioData DesertData;
    DesertData.AmbientVolume = 0.5f;
    DesertData.AudioRange = 25000.0f;
    BiomeAudioMap.Add(EBiomeType::Desert, DesertData);

    FAudio_BiomeAudioData MountainData;
    MountainData.AmbientVolume = 0.6f;
    MountainData.AudioRange = 18000.0f;
    BiomeAudioMap.Add(EBiomeType::Mountain, MountainData);
}

void AAudio_BiomeManager::OnAudioFinished()
{
    // Handle audio completion events
    UE_LOG(LogTemp, Log, TEXT("Biome audio finished playing"));
}

void AAudio_BiomeManager::UpdateAudioComponents()
{
    if (BiomeAudioMap.Contains(CurrentBiome))
    {
        const FAudio_BiomeAudioData& AudioData = BiomeAudioMap[CurrentBiome];
        
        // Update primary audio
        if (PrimaryAudioComponent && AudioData.PrimaryAmbientSound)
        {
            PrimaryAudioComponent->SetSound(AudioData.PrimaryAmbientSound);
            PrimaryAudioComponent->SetVolumeMultiplier(AudioData.AmbientVolume * MasterVolume);
            if (!PrimaryAudioComponent->IsPlaying())
            {
                PrimaryAudioComponent->Play();
            }
        }

        // Update secondary audio
        if (SecondaryAudioComponent && AudioData.SecondaryAmbientSound)
        {
            SecondaryAudioComponent->SetSound(AudioData.SecondaryAmbientSound);
            SecondaryAudioComponent->SetVolumeMultiplier(AudioData.AmbientVolume * 0.7f * MasterVolume);
            if (!SecondaryAudioComponent->IsPlaying())
            {
                SecondaryAudioComponent->Play();
            }
        }
    }
}

void AAudio_BiomeManager::FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float Duration)
{
    if (AudioComp)
    {
        AudioComp->FadeOut(Duration, TargetVolume);
    }
}

void AAudio_BiomeManager::SetupBiomeCenters()
{
    // Define biome center coordinates based on world layout
    BiomeCenters.Add(EBiomeType::Savanna, FVector(0.0f, 0.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Swamp, FVector(-50000.0f, -45000.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Forest, FVector(-45000.0f, 40000.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Desert, FVector(55000.0f, 0.0f, 0.0f));
    BiomeCenters.Add(EBiomeType::Mountain, FVector(40000.0f, 50000.0f, 0.0f));
}

float AAudio_BiomeManager::CalculateDistanceToBiomeCenter(EBiomeType Biome, FVector Location)
{
    if (BiomeCenters.Contains(Biome))
    {
        return FVector::Dist(Location, BiomeCenters[Biome]);
    }
    return FLT_MAX;
}