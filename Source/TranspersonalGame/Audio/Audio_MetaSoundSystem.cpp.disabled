#include "Audio_MetaSoundSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AAudio_MetaSoundSystem::AAudio_MetaSoundSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize Audio Components
    MasterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MasterAudioComponent"));
    RootComponent = MasterAudioComponent;

    BiomeAmbienceComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAmbienceComponent"));
    BiomeAmbienceComponent->SetupAttachment(RootComponent);

    DinosaurSFXComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DinosaurSFXComponent"));
    DinosaurSFXComponent->SetupAttachment(RootComponent);

    DialogueComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueComponent"));
    DialogueComponent->SetupAttachment(RootComponent);

    // Initialize Default Values
    CurrentBiome = EAudio_BiomeType::Savana;
    BiomeTransitionSpeed = 2.0f;
    EnvironmentalVolume = 0.8f;
    CurrentBiomeVolume = 1.0f;
    TargetBiomeVolume = 1.0f;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
}

void AAudio_MetaSoundSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize Audio Database with pre-generated content
    InitializeAudioDatabase();
    
    // Setup biome audio data
    SetupBiomeAudioData();
    SetupDinosaurAudioData();
    SetupDialogueAudioData();

    // Configure audio components
    if (BiomeAmbienceComponent)
    {
        BiomeAmbienceComponent->bAutoActivate = true;
        BiomeAmbienceComponent->SetVolumeMultiplier(EnvironmentalVolume);
    }

    if (DinosaurSFXComponent)
    {
        DinosaurSFXComponent->bAutoActivate = false;
        DinosaurSFXComponent->SetVolumeMultiplier(1.0f);
    }

    if (DialogueComponent)
    {
        DialogueComponent->bAutoActivate = false;
        DialogueComponent->SetVolumeMultiplier(0.9f);
    }

    UE_LOG(LogTemp, Warning, TEXT("Audio_MetaSoundSystem initialized with %d biomes"), BiomeAudioData.Num());
}

void AAudio_MetaSoundSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update player location-based audio
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            UpdateBiomeAudio(PlayerPawn->GetActorLocation());
        }
    }

    // Handle biome transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / BiomeTransitionSpeed, 0.0f, 1.0f);
        CurrentBiomeVolume = FMath::Lerp(CurrentBiomeVolume, TargetBiomeVolume, Alpha);

        if (BiomeAmbienceComponent)
        {
            BiomeAmbienceComponent->SetVolumeMultiplier(CurrentBiomeVolume * EnvironmentalVolume);
        }

        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }

    UpdateAudioComponents();
}

void AAudio_MetaSoundSystem::UpdateBiomeAudio(const FVector& PlayerLocation)
{
    // Determine current biome based on player location
    EAudio_BiomeType NewBiome = CurrentBiome;

    // Biome detection based on coordinates from memory ID 709
    if (PlayerLocation.X >= -55000 && PlayerLocation.X <= -45000 && PlayerLocation.Y >= -50000 && PlayerLocation.Y <= -40000)
    {
        NewBiome = EAudio_BiomeType::Pantano; // Swamp SW
    }
    else if (PlayerLocation.X >= -50000 && PlayerLocation.X <= -40000 && PlayerLocation.Y >= 35000 && PlayerLocation.Y <= 45000)
    {
        NewBiome = EAudio_BiomeType::Floresta; // Forest NW
    }
    else if (PlayerLocation.X >= 50000 && PlayerLocation.X <= 60000 && PlayerLocation.Y >= -5000 && PlayerLocation.Y <= 5000)
    {
        NewBiome = EAudio_BiomeType::Deserto; // Desert E
    }
    else if (PlayerLocation.X >= 35000 && PlayerLocation.X <= 45000 && PlayerLocation.Y >= 45000 && PlayerLocation.Y <= 55000)
    {
        NewBiome = EAudio_BiomeType::Montanha; // Mountain NE
    }
    else
    {
        NewBiome = EAudio_BiomeType::Savana; // Default Savana center
    }

    if (NewBiome != CurrentBiome)
    {
        TransitionToBiome(NewBiome);
    }
}

void AAudio_MetaSoundSystem::TransitionToBiome(EAudio_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome) return;

    UE_LOG(LogTemp, Warning, TEXT("Transitioning from biome %d to biome %d"), (int32)CurrentBiome, (int32)NewBiome);

    CurrentBiome = NewBiome;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
    TargetBiomeVolume = 1.0f;

    // Find biome audio data
    for (const FAudio_BiomeAmbience& BiomeData : BiomeAudioData)
    {
        if (BiomeData.BiomeType == NewBiome)
        {
            // TODO: Load and play biome-specific audio
            UE_LOG(LogTemp, Warning, TEXT("Playing audio for biome: %d with %d ambient sounds"), 
                   (int32)NewBiome, BiomeData.AmbientSounds.Num());
            break;
        }
    }
}

void AAudio_MetaSoundSystem::SetBiomeAmbience(EAudio_BiomeType BiomeType, const FAudio_SoundData& AmbienceData)
{
    for (FAudio_BiomeAmbience& BiomeData : BiomeAudioData)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            BiomeData.MusicTrack = AmbienceData;
            UE_LOG(LogTemp, Warning, TEXT("Set ambience for biome %d: %s"), (int32)BiomeType, *AmbienceData.SoundName);
            return;
        }
    }
}

void AAudio_MetaSoundSystem::PlayDinosaurRoar(EAudio_DinosaurType DinosaurType, const FVector& Location)
{
    if (FAudio_SoundData* RoarData = DinosaurRoars.Find(DinosaurType))
    {
        if (DinosaurSFXComponent)
        {
            DinosaurSFXComponent->SetWorldLocation(Location);
            // TODO: Load and play dinosaur roar audio from URL
            UE_LOG(LogTemp, Warning, TEXT("Playing %s roar at location (%f, %f, %f)"), 
                   *RoarData->SoundName, Location.X, Location.Y, Location.Z);
        }
    }
}

void AAudio_MetaSoundSystem::PlayDinosaurFootsteps(EAudio_DinosaurType DinosaurType, const FVector& Location)
{
    if (FAudio_SoundData* FootstepData = DinosaurFootsteps.Find(DinosaurType))
    {
        if (DinosaurSFXComponent)
        {
            DinosaurSFXComponent->SetWorldLocation(Location);
            // TODO: Load and play dinosaur footstep audio from URL
            UE_LOG(LogTemp, Warning, TEXT("Playing %s footsteps at location (%f, %f, %f)"), 
                   *FootstepData->SoundName, Location.X, Location.Y, Location.Z);
        }
    }
}

void AAudio_MetaSoundSystem::RegisterDinosaurAudio(EAudio_DinosaurType DinosaurType, const FAudio_SoundData& RoarData, const FAudio_SoundData& FootstepData)
{
    DinosaurRoars.Add(DinosaurType, RoarData);
    DinosaurFootsteps.Add(DinosaurType, FootstepData);
    UE_LOG(LogTemp, Warning, TEXT("Registered audio for dinosaur type %d"), (int32)DinosaurType);
}

void AAudio_MetaSoundSystem::PlayTribalDialogue(const FString& DialogueID, const FVector& SpeakerLocation)
{
    for (const FAudio_SoundData& DialogueData : TribalDialogue)
    {
        if (DialogueData.SoundName.Contains(DialogueID))
        {
            if (DialogueComponent)
            {
                DialogueComponent->SetWorldLocation(SpeakerLocation);
                CurrentDialogue = DialogueData;
                // TODO: Load and play dialogue audio from URL
                UE_LOG(LogTemp, Warning, TEXT("Playing tribal dialogue: %s at location (%f, %f, %f)"), 
                       *DialogueData.SoundName, SpeakerLocation.X, SpeakerLocation.Y, SpeakerLocation.Z);
            }
            break;
        }
    }
}

void AAudio_MetaSoundSystem::RegisterDialogueAudio(const FString& DialogueID, const FAudio_SoundData& DialogueData)
{
    TribalDialogue.Add(DialogueData);
    UE_LOG(LogTemp, Warning, TEXT("Registered dialogue audio: %s"), *DialogueID);
}

void AAudio_MetaSoundSystem::StopCurrentDialogue()
{
    if (DialogueComponent && DialogueComponent->IsPlaying())
    {
        DialogueComponent->Stop();
        UE_LOG(LogTemp, Warning, TEXT("Stopped current dialogue"));
    }
}

void AAudio_MetaSoundSystem::UpdateWeatherAudio(const FString& WeatherType, float Intensity)
{
    WeatherAmbience.SoundName = FString::Printf(TEXT("Weather_%s"), *WeatherType);
    WeatherAmbience.Volume = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Updated weather audio: %s with intensity %f"), *WeatherType, Intensity);
}

void AAudio_MetaSoundSystem::SetMasterVolume(float Volume)
{
    EnvironmentalVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (MasterAudioComponent)
    {
        MasterAudioComponent->SetVolumeMultiplier(EnvironmentalVolume);
    }
}

void AAudio_MetaSoundSystem::InitializeAudioDatabase()
{
    // Load pre-generated audio from previous cycles
    LoadPreGeneratedAudio();
    
    UE_LOG(LogTemp, Warning, TEXT("Audio database initialized with %d dialogue entries"), TribalDialogue.Num());
}

void AAudio_MetaSoundSystem::UpdateAudioComponents()
{
    // Update component states and 3D positioning
    if (BiomeAmbienceComponent && !BiomeAmbienceComponent->IsPlaying())
    {
        // TODO: Start biome ambience if not playing
    }
}

void AAudio_MetaSoundSystem::CalculateBiomeFromLocation(const FVector& Location)
{
    // This function is called by UpdateBiomeAudio
    // Implementation moved to UpdateBiomeAudio for clarity
}

void AAudio_MetaSoundSystem::FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime)
{
    if (Component)
    {
        // TODO: Implement smooth audio fading
        Component->SetVolumeMultiplier(TargetVolume);
    }
}

void AAudio_MetaSoundSystem::LoadPreGeneratedAudio()
{
    // Load dialogue audio from previous agent cycles
    FAudio_SoundData TribalElderAudio;
    TribalElderAudio.SoundName = TEXT("TribalElder_ThunderLizard");
    TribalElderAudio.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904906665_TribalElder.mp3");
    TribalElderAudio.Duration = 25.0f;
    TribalElderAudio.Volume = 0.9f;
    TribalElderAudio.bIs3D = true;
    TribalDialogue.Add(TribalElderAudio);

    FAudio_SoundData TribalWarriorAudio;
    TribalWarriorAudio.SoundName = TEXT("TribalWarrior_RaptorWarning");
    TribalWarriorAudio.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904912638_TribalWarrior.mp3");
    TribalWarriorAudio.Duration = 22.0f;
    TribalWarriorAudio.Volume = 0.9f;
    TribalWarriorAudio.bIs3D = true;
    TribalDialogue.Add(TribalWarriorAudio);

    FAudio_SoundData TribalScoutAudio;
    TribalScoutAudio.SoundName = TEXT("TribalScout_BrachiosaurusInfo");
    TribalScoutAudio.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904918646_TribalScout.mp3");
    TribalScoutAudio.Duration = 20.0f;
    TribalScoutAudio.Volume = 0.9f;
    TribalScoutAudio.bIs3D = true;
    TribalDialogue.Add(TribalScoutAudio);

    FAudio_SoundData TribalTrackerAudio;
    TribalTrackerAudio.SoundName = TEXT("TribalTracker_BloodWarning");
    TribalTrackerAudio.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778904924389_TribalTracker.mp3");
    TribalTrackerAudio.Duration = 18.0f;
    TribalTrackerAudio.Volume = 0.9f;
    TribalTrackerAudio.bIs3D = true;
    TribalDialogue.Add(TribalTrackerAudio);

    // Add current cycle audio
    FAudio_SoundData EnvironmentalNarrator;
    EnvironmentalNarrator.SoundName = TEXT("EnvironmentalNarrator_PrehistoricWorld");
    EnvironmentalNarrator.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778905039303_EnvironmentalNarrator.mp3");
    EnvironmentalNarrator.Duration = 21.0f;
    EnvironmentalNarrator.Volume = 0.8f;
    EnvironmentalNarrator.bIs3D = false;
    TribalDialogue.Add(EnvironmentalNarrator);

    FAudio_SoundData EmergencyWarning;
    EmergencyWarning.SoundName = TEXT("EmergencyWarning_TRexApproach");
    EmergencyWarning.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778905045237_EmergencyWarning.mp3");
    EmergencyWarning.Duration = 15.0f;
    EmergencyWarning.Volume = 1.0f;
    EmergencyWarning.bIs3D = true;
    TribalDialogue.Add(EmergencyWarning);
}

void AAudio_MetaSoundSystem::SetupBiomeAudioData()
{
    // Initialize biome audio configurations
    BiomeAudioData.Empty();

    // Savana Biome
    FAudio_BiomeAmbience SavanaBiome;
    SavanaBiome.BiomeType = EAudio_BiomeType::Savana;
    SavanaBiome.BiomeCenter = FVector(0, 0, 0);
    SavanaBiome.FadeDistance = 10000.0f;
    BiomeAudioData.Add(SavanaBiome);

    // Pantano Biome
    FAudio_BiomeAmbience PantanoBiome;
    PantanoBiome.BiomeType = EAudio_BiomeType::Pantano;
    PantanoBiome.BiomeCenter = FVector(-50000, -45000, 0);
    PantanoBiome.FadeDistance = 8000.0f;
    BiomeAudioData.Add(PantanoBiome);

    // Floresta Biome
    FAudio_BiomeAmbience FlorestaBiome;
    FlorestaBiome.BiomeType = EAudio_BiomeType::Floresta;
    FlorestaBiome.BiomeCenter = FVector(-45000, 40000, 0);
    FlorestaBiome.FadeDistance = 12000.0f;
    BiomeAudioData.Add(FlorestaBiome);

    // Deserto Biome
    FAudio_BiomeAmbience DesertoBiome;
    DesertoBiome.BiomeType = EAudio_BiomeType::Deserto;
    DesertoBiome.BiomeCenter = FVector(55000, 0, 0);
    DesertoBiome.FadeDistance = 15000.0f;
    BiomeAudioData.Add(DesertoBiome);

    // Montanha Biome
    FAudio_BiomeAmbience MontanhaBiome;
    MontanhaBiome.BiomeType = EAudio_BiomeType::Montanha;
    MontanhaBiome.BiomeCenter = FVector(40000, 50000, 0);
    MontanhaBiome.FadeDistance = 9000.0f;
    BiomeAudioData.Add(MontanhaBiome);
}

void AAudio_MetaSoundSystem::SetupDinosaurAudioData()
{
    // Setup T-Rex audio
    FAudio_SoundData TRexRoar;
    TRexRoar.SoundName = TEXT("TRex_Roar");
    TRexRoar.AudioURL = TEXT(""); // To be populated with Freesound assets
    TRexRoar.Duration = 5.0f;
    TRexRoar.Volume = 1.0f;
    TRexRoar.bIs3D = true;

    FAudio_SoundData TRexFootsteps;
    TRexFootsteps.SoundName = TEXT("TRex_Footsteps");
    TRexFootsteps.AudioURL = TEXT(""); // To be populated with Freesound assets
    TRexFootsteps.Duration = 2.0f;
    TRexFootsteps.Volume = 0.8f;
    TRexFootsteps.bIs3D = true;

    RegisterDinosaurAudio(EAudio_DinosaurType::TRex, TRexRoar, TRexFootsteps);

    // Setup Raptor audio
    FAudio_SoundData RaptorRoar;
    RaptorRoar.SoundName = TEXT("Raptor_Growl");
    RaptorRoar.AudioURL = TEXT(""); // To be populated with Freesound assets
    RaptorRoar.Duration = 3.0f;
    RaptorRoar.Volume = 0.7f;
    RaptorRoar.bIs3D = true;

    FAudio_SoundData RaptorFootsteps;
    RaptorFootsteps.SoundName = TEXT("Raptor_Footsteps");
    RaptorFootsteps.AudioURL = TEXT(""); // To be populated with Freesound assets
    RaptorFootsteps.Duration = 1.0f;
    RaptorFootsteps.Volume = 0.5f;
    RaptorFootsteps.bIs3D = true;

    RegisterDinosaurAudio(EAudio_DinosaurType::Raptor, RaptorRoar, RaptorFootsteps);

    // Setup Brachiosaurus audio
    FAudio_SoundData BrachiosaurusRoar;
    BrachiosaurusRoar.SoundName = TEXT("Brachiosaurus_Call");
    BrachiosaurusRoar.AudioURL = TEXT(""); // To be populated with Freesound assets
    BrachiosaurusRoar.Duration = 8.0f;
    BrachiosaurusRoar.Volume = 0.9f;
    BrachiosaurusRoar.bIs3D = true;

    FAudio_SoundData BrachiosaurusFootsteps;
    BrachiosaurusFootsteps.SoundName = TEXT("Brachiosaurus_Footsteps");
    BrachiosaurusFootsteps.AudioURL = TEXT(""); // To be populated with Freesound assets
    BrachiosaurusFootsteps.Duration = 3.0f;
    BrachiosaurusFootsteps.Volume = 1.0f;
    BrachiosaurusFootsteps.bIs3D = true;

    RegisterDinosaurAudio(EAudio_DinosaurType::Brachiosaurus, BrachiosaurusRoar, BrachiosaurusFootsteps);
}

void AAudio_MetaSoundSystem::SetupDialogueAudioData()
{
    // Dialogue audio is loaded in LoadPreGeneratedAudio()
    // This function can be used for additional dialogue setup
    UE_LOG(LogTemp, Warning, TEXT("Dialogue audio data setup complete"));
}