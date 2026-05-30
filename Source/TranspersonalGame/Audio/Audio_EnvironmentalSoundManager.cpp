#include "Audio_EnvironmentalSoundManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

AAudio_EnvironmentalSoundManager::AAudio_EnvironmentalSoundManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    CurrentBiomeAmbient = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAmbientAudio"));
    CurrentBiomeWind = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeWindAudio"));
    CurrentBiomeCreatures = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeCreatureAudio"));

    // Set up root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    CurrentBiomeAmbient->SetupAttachment(RootComponent);
    CurrentBiomeWind->SetupAttachment(RootComponent);
    CurrentBiomeCreatures->SetupAttachment(RootComponent);

    // Configure audio components
    CurrentBiomeAmbient->bAutoActivate = false;
    CurrentBiomeWind->bAutoActivate = false;
    CurrentBiomeCreatures->bAutoActivate = false;

    CurrentBiomeAmbient->SetVolumeMultiplier(0.8f);
    CurrentBiomeWind->SetVolumeMultiplier(0.6f);
    CurrentBiomeCreatures->SetVolumeMultiplier(0.7f);
}

void AAudio_EnvironmentalSoundManager::BeginPlay()
{
    Super::BeginPlay();

    InitializeBiomeAudioData();
    InitializeDinosaurAudioData();

    // Start with Savana biome audio
    UpdateBiomeAudio(EAudio_BiomeType::Savana);
}

void AAudio_EnvironmentalSoundManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle biome fade transitions
    if (bIsFading)
    {
        FadeTimer += DeltaTime;
        float FadeAlpha = FadeTimer / FadeDuration;

        if (FadeAlpha >= 1.0f)
        {
            // Fade complete
            bIsFading = false;
            FadeTimer = 0.0f;
            UpdateBiomeAudio(FadeToBiome);
        }
        else
        {
            // Update fade volumes
            float FromVolume = (1.0f - FadeAlpha) * MasterEnvironmentalVolume;
            float ToVolume = FadeAlpha * MasterEnvironmentalVolume;

            CurrentBiomeAmbient->SetVolumeMultiplier(FromVolume);
            CurrentBiomeWind->SetVolumeMultiplier(FromVolume * 0.6f);
            CurrentBiomeCreatures->SetVolumeMultiplier(FromVolume * 0.7f);
        }
    }

    // Update biome based on player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        EAudio_BiomeType DetectedBiome = GetBiomeFromLocation(PlayerLocation);

        if (DetectedBiome != CurrentBiome && !bIsFading)
        {
            FadeBetweenBiomes(CurrentBiome, DetectedBiome, 3.0f);
        }
    }
}

void AAudio_EnvironmentalSoundManager::InitializeBiomeAudioData()
{
    // Initialize biome audio data with placeholder paths
    // These will be replaced with actual sound cues when available

    FAudio_BiomeAudioData SavanaData;
    SavanaData.BaseVolume = 0.7f;
    SavanaData.FadeDistance = 8000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Savana, SavanaData);

    FAudio_BiomeAudioData PantanoData;
    PantanoData.BaseVolume = 0.8f;
    PantanoData.FadeDistance = 6000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Pantano, PantanoData);

    FAudio_BiomeAudioData FlorestaData;
    FlorestaData.BaseVolume = 0.9f;
    FlorestaData.FadeDistance = 7000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Floresta, FlorestaData);

    FAudio_BiomeAudioData DesertoData;
    DesertoData.BaseVolume = 0.6f;
    DesertoData.FadeDistance = 10000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Deserto, DesertoData);

    FAudio_BiomeAudioData MontanhaData;
    MontanhaData.BaseVolume = 0.8f;
    MontanhaData.FadeDistance = 9000.0f;
    BiomeAudioMap.Add(EAudio_BiomeType::Montanha, MontanhaData);
}

void AAudio_EnvironmentalSoundManager::InitializeDinosaurAudioData()
{
    // T-Rex audio data
    FAudio_DinosaurAudioData TRexData;
    TRexData.AudioRange = 5000.0f;
    TRexData.VolumeMultiplier = 1.2f;
    DinosaurAudioMap.Add(TEXT("TRex"), TRexData);

    // Velociraptor audio data
    FAudio_DinosaurAudioData VelociraptorData;
    VelociraptorData.AudioRange = 2500.0f;
    VelociraptorData.VolumeMultiplier = 0.8f;
    DinosaurAudioMap.Add(TEXT("Velociraptor"), VelociraptorData);

    // Brachiosaurus audio data
    FAudio_DinosaurAudioData BrachiosaurusData;
    BrachiosaurusData.AudioRange = 8000.0f;
    BrachiosaurusData.VolumeMultiplier = 1.5f;
    DinosaurAudioMap.Add(TEXT("Brachiosaurus"), BrachiosaurusData);

    // Triceratops audio data
    FAudio_DinosaurAudioData TriceratopsData;
    TriceratopsData.AudioRange = 4000.0f;
    TriceratopsData.VolumeMultiplier = 1.0f;
    DinosaurAudioMap.Add(TEXT("Triceratops"), TriceratopsData);

    // Ankylosaurus audio data
    FAudio_DinosaurAudioData AnkylosaurusData;
    AnkylosaurusData.AudioRange = 3500.0f;
    AnkylosaurusData.VolumeMultiplier = 0.9f;
    DinosaurAudioMap.Add(TEXT("Ankylosaurus"), AnkylosaurusData);
}

void AAudio_EnvironmentalSoundManager::UpdateBiomeAudio(EAudio_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;

    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(NewBiome))
    {
        // Stop current audio
        CurrentBiomeAmbient->Stop();
        CurrentBiomeWind->Stop();
        CurrentBiomeCreatures->Stop();

        // Set volumes based on biome data and master volume
        float AmbientVolume = BiomeData->BaseVolume * MasterEnvironmentalVolume;
        float WindVolume = BiomeData->BaseVolume * 0.6f * MasterEnvironmentalVolume;
        float CreatureVolume = BiomeData->BaseVolume * 0.7f * MasterEnvironmentalVolume;

        CurrentBiomeAmbient->SetVolumeMultiplier(AmbientVolume);
        CurrentBiomeWind->SetVolumeMultiplier(WindVolume);
        CurrentBiomeCreatures->SetVolumeMultiplier(CreatureVolume);

        // Load and play new sounds if available
        if (BiomeData->AmbientSound.IsValid())
        {
            CurrentBiomeAmbient->SetSound(BiomeData->AmbientSound.Get());
            CurrentBiomeAmbient->Play();
        }

        if (BiomeData->WindSound.IsValid())
        {
            CurrentBiomeWind->SetSound(BiomeData->WindSound.Get());
            CurrentBiomeWind->Play();
        }

        if (BiomeData->CreatureSound.IsValid())
        {
            CurrentBiomeCreatures->SetSound(BiomeData->CreatureSound.Get());
            CurrentBiomeCreatures->Play();
        }

        UE_LOG(LogTemp, Log, TEXT("Environmental Audio: Switched to biome %d"), (int32)NewBiome);
    }
}

EAudio_BiomeType AAudio_EnvironmentalSoundManager::GetBiomeFromLocation(FVector Location)
{
    // Biome detection based on coordinates from memory
    // Savana (0,0), Pantano (-50000,-45000), Floresta (-45000,40000), 
    // Deserto (55000,0), Montanha (40000,50000)

    float X = Location.X;
    float Y = Location.Y;

    // Calculate distances to biome centers
    float DistToSavana = FVector::Dist2D(Location, FVector(0, 0, 0));
    float DistToPantano = FVector::Dist2D(Location, FVector(-50000, -45000, 0));
    float DistToFloresta = FVector::Dist2D(Location, FVector(-45000, 40000, 0));
    float DistToDeserto = FVector::Dist2D(Location, FVector(55000, 0, 0));
    float DistToMontanha = FVector::Dist2D(Location, FVector(40000, 50000, 0));

    // Find closest biome
    float MinDistance = DistToSavana;
    EAudio_BiomeType ClosestBiome = EAudio_BiomeType::Savana;

    if (DistToPantano < MinDistance)
    {
        MinDistance = DistToPantano;
        ClosestBiome = EAudio_BiomeType::Pantano;
    }

    if (DistToFloresta < MinDistance)
    {
        MinDistance = DistToFloresta;
        ClosestBiome = EAudio_BiomeType::Floresta;
    }

    if (DistToDeserto < MinDistance)
    {
        MinDistance = DistToDeserto;
        ClosestBiome = EAudio_BiomeType::Deserto;
    }

    if (DistToMontanha < MinDistance)
    {
        MinDistance = DistToMontanha;
        ClosestBiome = EAudio_BiomeType::Montanha;
    }

    return ClosestBiome;
}

void AAudio_EnvironmentalSoundManager::PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType, FVector Location, float VolumeOverride)
{
    if (FAudio_DinosaurAudioData* DinoData = DinosaurAudioMap.Find(DinosaurType))
    {
        // Calculate distance to player for volume adjustment
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (!PlayerPawn) return;

        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
        if (Distance > DinoData->AudioRange) return; // Too far to hear

        // Calculate volume based on distance
        float VolumeMultiplier = VolumeOverride >= 0.0f ? VolumeOverride : DinoData->VolumeMultiplier;
        float DistanceVolume = 1.0f - (Distance / DinoData->AudioRange);
        float FinalVolume = VolumeMultiplier * DistanceVolume * MasterDinosaurVolume;

        // Determine which sound to play
        USoundCue* SoundToPlay = nullptr;
        if (SoundType == TEXT("Idle") && DinoData->IdleSound.IsValid())
        {
            SoundToPlay = DinoData->IdleSound.Get();
        }
        else if (SoundType == TEXT("Movement") && DinoData->MovementSound.IsValid())
        {
            SoundToPlay = DinoData->MovementSound.Get();
        }
        else if (SoundType == TEXT("Aggressive") && DinoData->AggressiveSound.IsValid())
        {
            SoundToPlay = DinoData->AggressiveSound.Get();
        }

        if (SoundToPlay)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, Location, FinalVolume);
            UE_LOG(LogTemp, Log, TEXT("Played %s %s sound at distance %.2f with volume %.2f"), 
                   *DinosaurType, *SoundType, Distance, FinalVolume);
        }
    }
}

void AAudio_EnvironmentalSoundManager::SetMasterEnvironmentalVolume(float NewVolume)
{
    MasterEnvironmentalVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    
    // Update current playing audio volumes
    if (FAudio_BiomeAudioData* BiomeData = BiomeAudioMap.Find(CurrentBiome))
    {
        CurrentBiomeAmbient->SetVolumeMultiplier(BiomeData->BaseVolume * MasterEnvironmentalVolume);
        CurrentBiomeWind->SetVolumeMultiplier(BiomeData->BaseVolume * 0.6f * MasterEnvironmentalVolume);
        CurrentBiomeCreatures->SetVolumeMultiplier(BiomeData->BaseVolume * 0.7f * MasterEnvironmentalVolume);
    }
}

void AAudio_EnvironmentalSoundManager::SetMasterDinosaurVolume(float NewVolume)
{
    MasterDinosaurVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
}

void AAudio_EnvironmentalSoundManager::FadeBetweenBiomes(EAudio_BiomeType FromBiome, EAudio_BiomeType ToBiome, float FadeTime)
{
    if (bIsFading) return; // Already fading

    bIsFading = true;
    FadeTimer = 0.0f;
    FadeDuration = FadeTime;
    FadeFromBiome = FromBiome;
    FadeToBiome = ToBiome;

    UE_LOG(LogTemp, Log, TEXT("Environmental Audio: Starting fade from biome %d to %d over %.2f seconds"), 
           (int32)FromBiome, (int32)ToBiome, FadeTime);
}

void AAudio_EnvironmentalSoundManager::UpdateVolumeBasedOnDistance(UAudioComponent* AudioComp, float Distance, float MaxDistance)
{
    if (!AudioComp) return;

    float VolumeMultiplier = 1.0f - FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
    AudioComp->SetVolumeMultiplier(VolumeMultiplier * MasterEnvironmentalVolume);
}