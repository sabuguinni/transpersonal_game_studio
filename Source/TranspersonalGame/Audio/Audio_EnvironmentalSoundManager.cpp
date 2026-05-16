#include "Audio_EnvironmentalSoundManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"

AAudio_EnvironmentalSoundManager::AAudio_EnvironmentalSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    RootComponent = AmbientAudioComponent;
    AmbientAudioComponent->bAutoActivate = false;
    AmbientAudioComponent->SetVolumeMultiplier(0.7f);

    RandomSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RandomSoundComponent"));
    RandomSoundComponent->SetupAttachment(RootComponent);
    RandomSoundComponent->bAutoActivate = false;
    RandomSoundComponent->SetVolumeMultiplier(0.5f);

    // Initialize defaults
    CurrentBiome = EAudio_BiomeType::Savanna;
    BiomeDetectionRadius = 5000.0f;
    MasterVolume = 1.0f;
}

void AAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeAudioData();
    UpdateAmbientAudio();
    StartRandomSoundTimer();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_EnvironmentalSoundManager: BeginPlay completed"));
}

void AAudio_EnvironmentalSoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopRandomSoundTimer();
    
    if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Stop();
    }
    
    Super::EndPlay(EndPlayReason);
}

void AAudio_EnvironmentalSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get player location for biome detection
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            EAudio_BiomeType DetectedBiome = DetectBiomeFromLocation(PlayerLocation);
            
            if (DetectedBiome != CurrentBiome)
            {
                SetCurrentBiome(DetectedBiome);
            }
        }
    }
}

void AAudio_EnvironmentalSoundManager::InitializeBiomeAudioData()
{
    // Initialize default biome audio data
    // Note: Sound assets will be assigned in Blueprint or via content loading
    
    FAudio_BiomeAudioData SavannaData;
    SavannaData.RandomSoundInterval = 20.0f;
    SavannaData.AmbientVolume = 0.6f;
    BiomeAudioMap.Add(EAudio_BiomeType::Savanna, SavannaData);
    
    FAudio_BiomeAudioData SwampData;
    SwampData.RandomSoundInterval = 15.0f;
    SwampData.AmbientVolume = 0.8f;
    BiomeAudioMap.Add(EAudio_BiomeType::Swamp, SwampData);
    
    FAudio_BiomeAudioData ForestData;
    ForestData.RandomSoundInterval = 12.0f;
    ForestData.AmbientVolume = 0.7f;
    BiomeAudioMap.Add(EAudio_BiomeType::Forest, ForestData);
    
    FAudio_BiomeAudioData DesertData;
    DesertData.RandomSoundInterval = 25.0f;
    DesertData.AmbientVolume = 0.5f;
    BiomeAudioMap.Add(EAudio_BiomeType::Desert, DesertData);
    
    FAudio_BiomeAudioData MountainData;
    MountainData.RandomSoundInterval = 30.0f;
    MountainData.AmbientVolume = 0.6f;
    BiomeAudioMap.Add(EAudio_BiomeType::Mountain, MountainData);
    
    UE_LOG(LogTemp, Log, TEXT("Audio_EnvironmentalSoundManager: Biome audio data initialized"));
}

void AAudio_EnvironmentalSoundManager::SetCurrentBiome(EAudio_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome)
    {
        return;
    }
    
    CurrentBiome = NewBiome;
    UpdateAmbientAudio();
    
    // Restart random sound timer with new interval
    StopRandomSoundTimer();
    StartRandomSoundTimer();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_EnvironmentalSoundManager: Biome changed to %d"), (int32)CurrentBiome);
}

void AAudio_EnvironmentalSoundManager::UpdateAmbientAudio()
{
    if (!AmbientAudioComponent)
    {
        return;
    }
    
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome))
    {
        if (BiomeData->AmbientLoop)
        {
            AmbientAudioComponent->SetSound(BiomeData->AmbientLoop);
            AmbientAudioComponent->SetVolumeMultiplier(BiomeData->AmbientVolume * MasterVolume);
            
            if (!AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->Play();
            }
        }
        else
        {
            // No ambient sound for this biome, stop current ambient
            if (AmbientAudioComponent->IsPlaying())
            {
                AmbientAudioComponent->Stop();
            }
        }
    }
}

void AAudio_EnvironmentalSoundManager::PlayRandomBiomeSound()
{
    if (!RandomSoundComponent)
    {
        return;
    }
    
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome))
    {
        if (BiomeData->RandomSounds.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, BiomeData->RandomSounds.Num() - 1);
            USoundCue* RandomSound = BiomeData->RandomSounds[RandomIndex];
            
            if (RandomSound)
            {
                RandomSoundComponent->SetSound(RandomSound);
                RandomSoundComponent->SetVolumeMultiplier(0.5f * MasterVolume);
                RandomSoundComponent->Play();
                
                UE_LOG(LogTemp, Log, TEXT("Audio_EnvironmentalSoundManager: Playing random sound %d for biome %d"), 
                       RandomIndex, (int32)CurrentBiome);
            }
        }
    }
}

EAudio_BiomeType AAudio_EnvironmentalSoundManager::DetectBiomeFromLocation(FVector Location)
{
    // Biome detection based on world coordinates
    // Coordinates from memory ID 709
    
    if (IsInSwamp(Location))
    {
        return EAudio_BiomeType::Swamp;
    }
    else if (IsInForest(Location))
    {
        return EAudio_BiomeType::Forest;
    }
    else if (IsInDesert(Location))
    {
        return EAudio_BiomeType::Desert;
    }
    else if (IsInMountain(Location))
    {
        return EAudio_BiomeType::Mountain;
    }
    else
    {
        return EAudio_BiomeType::Savanna; // Default/center biome
    }
}

void AAudio_EnvironmentalSoundManager::SetMasterVolume(float NewVolume)
{
    MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    UpdateAmbientAudio();
    
    UE_LOG(LogTemp, Log, TEXT("Audio_EnvironmentalSoundManager: Master volume set to %f"), MasterVolume);
}

void AAudio_EnvironmentalSoundManager::StartRandomSoundTimer()
{
    if (UWorld* World = GetWorld())
    {
        if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome))
        {
            World->GetTimerManager().SetTimer(
                RandomSoundTimer,
                this,
                &AAudio_EnvironmentalSoundManager::PlayRandomBiomeSound,
                BiomeData->RandomSoundInterval,
                true
            );
        }
    }
}

void AAudio_EnvironmentalSoundManager::StopRandomSoundTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RandomSoundTimer);
    }
}

// Biome detection helper functions based on world coordinates
bool AAudio_EnvironmentalSoundManager::IsInSavanna(FVector Location)
{
    // Savanna is the center biome (0, 0)
    return FVector::Dist2D(Location, FVector(0, 0, 0)) < BiomeDetectionRadius;
}

bool AAudio_EnvironmentalSoundManager::IsInSwamp(FVector Location)
{
    // Swamp coordinates: X=-50000, Y=-45000
    return FVector::Dist2D(Location, FVector(-50000, -45000, 0)) < BiomeDetectionRadius;
}

bool AAudio_EnvironmentalSoundManager::IsInForest(FVector Location)
{
    // Forest coordinates: X=-45000, Y=40000
    return FVector::Dist2D(Location, FVector(-45000, 40000, 0)) < BiomeDetectionRadius;
}

bool AAudio_EnvironmentalSoundManager::IsInDesert(FVector Location)
{
    // Desert coordinates: X=55000, Y=0
    return FVector::Dist2D(Location, FVector(55000, 0, 0)) < BiomeDetectionRadius;
}

bool AAudio_EnvironmentalSoundManager::IsInMountain(FVector Location)
{
    // Mountain coordinates: X=40000, Y=50000
    return FVector::Dist2D(Location, FVector(40000, 50000, 0)) < BiomeDetectionRadius;
}