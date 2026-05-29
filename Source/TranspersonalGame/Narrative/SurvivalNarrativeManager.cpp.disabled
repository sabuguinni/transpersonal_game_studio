#include "SurvivalNarrativeManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

USurvivalNarrativeManager::USurvivalNarrativeManager()
{
    NarrativeAudioComponent = nullptr;
    NarrativeVolume = 0.8f;
    TriggerCheckInterval = 1.0f;
    bNarrativeEnabled = true;
    bIsInitialized = false;
}

void USurvivalNarrativeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalNarrativeManager: Initializing narrative system"));
    
    // Initialize survival tips database
    InitializeSurvivalTips();
    
    // Setup default narrative triggers
    SetupDefaultTriggers();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalNarrativeManager: Initialization complete"));
}

void USurvivalNarrativeManager::Deinitialize()
{
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(TriggerCheckTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(TriggerCheckTimer);
    }
    
    if (NarrativeAudioComponent && IsValid(NarrativeAudioComponent))
    {
        NarrativeAudioComponent->Stop();
        NarrativeAudioComponent = nullptr;
    }
    
    Super::Deinitialize();
}

void USurvivalNarrativeManager::InitializeSurvivalTips()
{
    SurvivalTips.Empty();
    
    // Water survival tips
    SurvivalTips.Add(CreateSurvivalTip(
        TEXT("Follow game trails to find water, but avoid muddy banks where predators drink"),
        TEXT("WaterGuide.mp3"),
        ENarr_SurvivalContext::Water,
        3.0f
    ));
    
    // Predator awareness tips
    SurvivalTips.Add(CreateSurvivalTip(
        TEXT("Listen for the deep rumble - that is the tyrant king's footstep. When you hear it, run"),
        TEXT("TribalElder.mp3"),
        ENarr_SurvivalContext::Danger,
        5.0f
    ));
    
    // Pack hunter warnings
    SurvivalTips.Add(CreateSurvivalTip(
        TEXT("The pack moves as one mind. Three shadows circling, waiting for weakness"),
        TEXT("SurvivalNarrator.mp3"),
        ENarr_SurvivalContext::Danger,
        4.5f
    ));
    
    // Gentle giant encounters
    SurvivalTips.Add(CreateSurvivalTip(
        TEXT("Move slowly near the long-necks. Those tree-trunk legs crush without the beast noticing"),
        TEXT("DinosaurObserver.mp3"),
        ENarr_SurvivalContext::Wildlife,
        3.5f
    ));
    
    // General survival wisdom
    SurvivalTips.Add(CreateSurvivalTip(
        TEXT("Stay close to fire at night. The darkness brings hunters your ancestors learned to fear"),
        TEXT("TribalElder.mp3"),
        ENarr_SurvivalContext::General,
        2.5f
    ));
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalNarrativeManager: Loaded %d survival tips"), SurvivalTips.Num());
}

void USurvivalNarrativeManager::RegisterNarrativeTrigger(const FNarr_NarrativeTrigger& Trigger)
{
    NarrativeTriggers.Add(Trigger);
    UE_LOG(LogTemp, Log, TEXT("SurvivalNarrativeManager: Registered trigger at location %s"), 
           *Trigger.Location.ToString());
}

void USurvivalNarrativeManager::CheckPlayerProximity(const FVector& PlayerLocation)
{
    if (!bNarrativeEnabled || !bIsInitialized)
    {
        return;
    }
    
    for (FNarr_NarrativeTrigger& Trigger : NarrativeTriggers)
    {
        if (Trigger.bIsOneTime && Trigger.bHasBeenTriggered)
        {
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, Trigger.Location);
        if (Distance <= Trigger.TriggerRadius)
        {
            TriggerSurvivalTip(Trigger.SurvivalTip);
            Trigger.bHasBeenTriggered = true;
            
            UE_LOG(LogTemp, Warning, TEXT("SurvivalNarrativeManager: Triggered narrative at distance %.2f"), Distance);
        }
    }
}

void USurvivalNarrativeManager::TriggerSurvivalTip(const FNarr_SurvivalTip& Tip)
{
    if (!bNarrativeEnabled || Tip.bIsTriggered)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalNarrativeManager: Triggering tip: %s"), *Tip.TipText);
    
    // Play audio if available
    if (!Tip.AudioFile.IsEmpty())
    {
        PlayNarrativeAudio(Tip.AudioFile);
    }
    
    // Mark as triggered (for non-repeatable tips)
    const_cast<FNarr_SurvivalTip&>(Tip).bIsTriggered = true;
}

void USurvivalNarrativeManager::PlayNarrativeAudio(const FString& AudioFile)
{
    if (AudioFile.IsEmpty())
    {
        return;
    }
    
    // Stop current audio if playing
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
    }
    
    // Create audio component if needed
    if (!NarrativeAudioComponent)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            NarrativeAudioComponent = UGameplayStatics::CreateSound2D(World, nullptr);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalNarrativeManager: Playing audio file %s"), *AudioFile);
}

void USurvivalNarrativeManager::TriggerContextualTip(ENarr_SurvivalContext Context)
{
    FNarr_SurvivalTip SelectedTip = GetRandomTipForContext(Context);
    if (!SelectedTip.TipText.IsEmpty())
    {
        TriggerSurvivalTip(SelectedTip);
    }
}

FNarr_SurvivalTip USurvivalNarrativeManager::GetRandomTipForContext(ENarr_SurvivalContext Context)
{
    TArray<FNarr_SurvivalTip> ContextTips;
    
    for (const FNarr_SurvivalTip& Tip : SurvivalTips)
    {
        if (Tip.Context == Context && !Tip.bIsTriggered)
        {
            ContextTips.Add(Tip);
        }
    }
    
    if (ContextTips.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, ContextTips.Num() - 1);
        return ContextTips[RandomIndex];
    }
    
    return FNarr_SurvivalTip();
}

void USurvivalNarrativeManager::SetNarrativeVolume(float Volume)
{
    NarrativeVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    
    if (NarrativeAudioComponent)
    {
        NarrativeAudioComponent->SetVolumeMultiplier(NarrativeVolume);
    }
}

bool USurvivalNarrativeManager::IsNarrativePlaying() const
{
    return NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying();
}

void USurvivalNarrativeManager::StopCurrentNarrative()
{
    if (NarrativeAudioComponent && NarrativeAudioComponent->IsPlaying())
    {
        NarrativeAudioComponent->Stop();
        UE_LOG(LogTemp, Log, TEXT("SurvivalNarrativeManager: Stopped current narrative audio"));
    }
}

void USurvivalNarrativeManager::LoadSurvivalTipDatabase()
{
    // Extended database loading - can be expanded with external data sources
    UE_LOG(LogTemp, Log, TEXT("SurvivalNarrativeManager: Loading extended tip database"));
}

void USurvivalNarrativeManager::SetupDefaultTriggers()
{
    // Water source trigger
    FNarr_NarrativeTrigger WaterTrigger;
    WaterTrigger.Location = FVector(500, 0, 100);
    WaterTrigger.TriggerRadius = 400.0f;
    WaterTrigger.SurvivalTip = GetRandomTipForContext(ENarr_SurvivalContext::Water);
    WaterTrigger.bIsOneTime = true;
    RegisterNarrativeTrigger(WaterTrigger);
    
    // Forest danger zone
    FNarr_NarrativeTrigger ForestTrigger;
    ForestTrigger.Location = FVector(-300, 400, 150);
    ForestTrigger.TriggerRadius = 350.0f;
    ForestTrigger.SurvivalTip = GetRandomTipForContext(ENarr_SurvivalContext::Danger);
    ForestTrigger.bIsOneTime = true;
    RegisterNarrativeTrigger(ForestTrigger);
    
    // Open plains wildlife area
    FNarr_NarrativeTrigger PlainsTrigger;
    PlainsTrigger.Location = FVector(200, -500, 120);
    PlainsTrigger.TriggerRadius = 500.0f;
    PlainsTrigger.SurvivalTip = GetRandomTipForContext(ENarr_SurvivalContext::Wildlife);
    PlainsTrigger.bIsOneTime = true;
    RegisterNarrativeTrigger(PlainsTrigger);
    
    // Rocky outcrop general survival
    FNarr_NarrativeTrigger RockTrigger;
    RockTrigger.Location = FVector(-600, -200, 180);
    RockTrigger.TriggerRadius = 300.0f;
    RockTrigger.SurvivalTip = GetRandomTipForContext(ENarr_SurvivalContext::General);
    RockTrigger.bIsOneTime = false; // Repeatable general tips
    RegisterNarrativeTrigger(RockTrigger);
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalNarrativeManager: Setup %d default triggers"), NarrativeTriggers.Num());
}

FNarr_SurvivalTip USurvivalNarrativeManager::CreateSurvivalTip(const FString& Text, const FString& Audio, ENarr_SurvivalContext Context, float Priority)
{
    FNarr_SurvivalTip NewTip;
    NewTip.TipText = Text;
    NewTip.AudioFile = Audio;
    NewTip.Context = Context;
    NewTip.Priority = Priority;
    NewTip.bIsTriggered = false;
    return NewTip;
}