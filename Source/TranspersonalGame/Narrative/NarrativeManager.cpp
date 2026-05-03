#include "NarrativeManager.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ANarrativeManager::ANarrativeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f;

    // Criar componente raiz
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Criar componente de áudio
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Configurações padrão
    CheckInterval = 2.0f;
    MaxNarrativeDistance = 2000.0f;
    bEnableDebugOutput = true;
    LastCheckTime = 0.0f;
    PlayerPawn = nullptr;
    bIsPlayingNarrative = false;
}

void ANarrativeManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar o jogador
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Inicializar base de dados de narrativa
    InitializeNarrativeDatabase();
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Initialized with %d narrative entries"), NarrativeEntries.Num());
    }
}

void ANarrativeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Verificar se é hora de verificar triggers
    if (CurrentTime - LastCheckTime >= CheckInterval)
    {
        CheckNarrativeTriggers();
        LastCheckTime = CurrentTime;
    }
}

void ANarrativeManager::InitializeNarrativeDatabase()
{
    NarrativeEntries.Empty();

    // Entry 1: Research Log - Territorial Boundary
    FNarr_NarrativeEntry Entry1;
    Entry1.EntryID = TEXT("research_log_001");
    Entry1.Type = ENarr_NarrativeType::ResearchLog;
    Entry1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777768524927_FieldResearcher.mp3");
    Entry1.TranscriptText = TEXT("Research log entry 47. The pack hunters have established a territorial boundary near the southern ridge. Their coordinated behavior suggests advanced social intelligence. Maintaining safe distance of at least 200 meters.");
    Entry1.TriggerCondition = ENarr_TriggerCondition::PlayerLocation;
    Entry1.TriggerLocation = FVector(1000.0f, -1500.0f, 100.0f);
    Entry1.TriggerRadius = 800.0f;
    Entry1.Cooldown = 120.0f;
    NarrativeEntries.Add(Entry1);

    // Entry 2: Safety Warning - Herbivore Herd
    FNarr_NarrativeEntry Entry2;
    Entry2.EntryID = TEXT("safety_warning_001");
    Entry2.Type = ENarr_NarrativeType::SafetyWarning;
    Entry2.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777768527197_SafetyNarrator.mp3");
    Entry2.TranscriptText = TEXT("Warning! Large herbivore herd detected moving through the valley. These massive creatures can trample anything in their path. Seek immediate shelter behind rock formations or elevated terrain.");
    Entry2.TriggerCondition = ENarr_TriggerCondition::DinosaurProximity;
    Entry2.TriggerLocation = FVector(-500.0f, 1000.0f, 50.0f);
    Entry2.TriggerRadius = 1000.0f;
    Entry2.Cooldown = 180.0f;
    NarrativeEntries.Add(Entry2);

    // Entry 3: Story Narration - Cretaceous Period
    FNarr_NarrativeEntry Entry3;
    Entry3.EntryID = TEXT("story_narration_001");
    Entry3.Type = ENarr_NarrativeType::StoryNarration;
    Entry3.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777768529369_StoryNarrator.mp3");
    Entry3.TranscriptText = TEXT("The Cretaceous period was a time of giants. Massive predators ruled the land, while herbivores traveled in protective herds. Every step could be your last in this ancient world.");
    Entry3.TriggerCondition = ENarr_TriggerCondition::FirstVisit;
    Entry3.TriggerLocation = FVector::ZeroVector;
    Entry3.TriggerRadius = 500.0f;
    Entry3.Cooldown = 300.0f;
    NarrativeEntries.Add(Entry3);

    // Entry 4: Survival Guide - Behavior Patterns
    FNarr_NarrativeEntry Entry4;
    Entry4.EntryID = TEXT("survival_guide_001");
    Entry4.Type = ENarr_NarrativeType::SurvivalGuide;
    Entry4.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777768531344_SurvivalGuide.mp3");
    Entry4.TranscriptText = TEXT("Survival depends on understanding dinosaur behavior. Predators hunt in patterns. Herbivores flee in predictable directions. Learn these patterns, or become prey yourself.");
    Entry4.TriggerCondition = ENarr_TriggerCondition::PlayerLocation;
    Entry4.TriggerLocation = FVector(500.0f, 500.0f, 200.0f);
    Entry4.TriggerRadius = 600.0f;
    Entry4.Cooldown = 240.0f;
    NarrativeEntries.Add(Entry4);

    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Database initialized with %d entries"), NarrativeEntries.Num());
    }
}

void ANarrativeManager::CheckNarrativeTriggers()
{
    if (!PlayerPawn || bIsPlayingNarrative)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Verificar triggers baseados em localização
    CheckLocationTriggers();
    
    // Verificar triggers baseados em proximidade
    CheckProximityTriggers();
    
    // Verificar triggers baseados em saúde
    CheckHealthTriggers();
    
    // Verificar triggers baseados em hora do dia
    CheckTimeOfDayTriggers();
}

void ANarrativeManager::CheckLocationTriggers()
{
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FNarr_NarrativeEntry& Entry : NarrativeEntries)
    {
        if (Entry.TriggerCondition == ENarr_TriggerCondition::PlayerLocation && 
            IsNarrativeEntryAvailable(Entry))
        {
            float Distance = FVector::Dist(PlayerLocation, Entry.TriggerLocation);
            
            if (Distance <= Entry.TriggerRadius)
            {
                if (PlayNarrativeEntry(Entry))
                {
                    OnNarrativeTriggered(Entry.TriggerCondition, Entry.TriggerLocation);
                    break; // Tocar apenas uma narrativa de cada vez
                }
            }
        }
    }
}

void ANarrativeManager::CheckProximityTriggers()
{
    if (!PlayerPawn)
    {
        return;
    }

    // Implementar verificação de proximidade de dinossauros
    // Por agora, usar localização como proxy
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FNarr_NarrativeEntry& Entry : NarrativeEntries)
    {
        if (Entry.TriggerCondition == ENarr_TriggerCondition::DinosaurProximity && 
            IsNarrativeEntryAvailable(Entry))
        {
            float Distance = FVector::Dist(PlayerLocation, Entry.TriggerLocation);
            
            if (Distance <= Entry.TriggerRadius)
            {
                if (PlayNarrativeEntry(Entry))
                {
                    OnNarrativeTriggered(Entry.TriggerCondition, PlayerLocation);
                    break;
                }
            }
        }
    }
}

void ANarrativeManager::CheckHealthTriggers()
{
    // Implementar verificação de estado de saúde
    // Requer integração com sistema de sobrevivência
}

void ANarrativeManager::CheckTimeOfDayTriggers()
{
    // Implementar verificação de hora do dia
    // Requer sistema de ciclo dia/noite
}

bool ANarrativeManager::PlayNarrativeEntry(const FNarr_NarrativeEntry& Entry)
{
    if (!CanPlayNarrative(Entry) || bIsPlayingNarrative)
    {
        return false;
    }

    // Encontrar a entrada na array para atualizar
    FNarr_NarrativeEntry* MutableEntry = FindNarrativeEntry(Entry.EntryID);
    if (!MutableEntry)
    {
        return false;
    }

    // Atualizar estado da entrada
    UpdateNarrativeState(*MutableEntry);
    
    // Marcar como a tocar
    bIsPlayingNarrative = true;
    
    // Evento Blueprint
    OnNarrativeStarted(Entry);
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Playing narrative '%s' - %s"), 
               *Entry.EntryID, *Entry.TranscriptText);
        
        // Mostrar na tela também
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                FString::Printf(TEXT("NARRATIVE: %s"), *Entry.TranscriptText));
        }
    }
    
    // Simular duração do áudio (em jogo real, usar evento de fim de áudio)
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        [this, Entry]()
        {
            bIsPlayingNarrative = false;
            OnNarrativeFinished(Entry);
        },
        15.0f, // Duração estimada
        false
    );
    
    return true;
}

void ANarrativeManager::StopCurrentNarrative()
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        AudioComponent->Stop();
    }
    
    bIsPlayingNarrative = false;
}

void ANarrativeManager::AddNarrativeEntry(const FNarr_NarrativeEntry& NewEntry)
{
    NarrativeEntries.Add(NewEntry);
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Added narrative entry '%s'"), *NewEntry.EntryID);
    }
}

TArray<FNarr_NarrativeEntry> ANarrativeManager::GetNarrativeEntriesInRadius(FVector Location, float Radius)
{
    TArray<FNarr_NarrativeEntry> Result;
    
    for (const FNarr_NarrativeEntry& Entry : NarrativeEntries)
    {
        float Distance = FVector::Dist(Location, Entry.TriggerLocation);
        if (Distance <= Radius)
        {
            Result.Add(Entry);
        }
    }
    
    return Result;
}

bool ANarrativeManager::IsNarrativeEntryAvailable(const FNarr_NarrativeEntry& Entry)
{
    return CanPlayNarrative(Entry);
}

void ANarrativeManager::ResetNarrativeEntry(const FString& EntryID)
{
    FNarr_NarrativeEntry* Entry = FindNarrativeEntry(EntryID);
    if (Entry)
    {
        Entry->bHasPlayed = false;
        Entry->LastPlayedTime = 0.0f;
        
        if (bEnableDebugOutput)
        {
            UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Reset narrative entry '%s'"), *EntryID);
        }
    }
}

void ANarrativeManager::ResetAllNarrativeEntries()
{
    for (FNarr_NarrativeEntry& Entry : NarrativeEntries)
    {
        Entry.bHasPlayed = false;
        Entry.LastPlayedTime = 0.0f;
    }
    
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeManager: Reset all narrative entries"));
    }
}

FNarr_NarrativeEntry* ANarrativeManager::FindNarrativeEntry(const FString& EntryID)
{
    for (FNarr_NarrativeEntry& Entry : NarrativeEntries)
    {
        if (Entry.EntryID == EntryID)
        {
            return &Entry;
        }
    }
    return nullptr;
}

bool ANarrativeManager::CanPlayNarrative(const FNarr_NarrativeEntry& Entry)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Verificar cooldown
    if (Entry.bHasPlayed && (CurrentTime - Entry.LastPlayedTime) < Entry.Cooldown)
    {
        return false;
    }
    
    // Verificar se já está a tocar narrativa
    if (bIsPlayingNarrative)
    {
        return false;
    }
    
    return true;
}

void ANarrativeManager::UpdateNarrativeState(FNarr_NarrativeEntry& Entry)
{
    Entry.bHasPlayed = true;
    Entry.LastPlayedTime = GetWorld()->GetTimeSeconds();
}