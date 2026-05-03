#include "NarrativeDialogueManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Criar componente de áudio
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
    RootComponent = DialogueAudioComponent;
    
    // Configurações padrão
    MaxDialogueDistance = 2000.0f;
    VolumeMultiplier = 1.0f;
    bAutoTriggerDialogues = true;
    MaxConcurrentDialogues = 1;
    
    // Estado inicial
    bIsDialoguePlaying = false;
    CachedPlayer = nullptr;
    ProximityCheckTimer = 0.0f;
    ContextCheckTimer = 0.0f;
}

void ANarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache do player
    CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Configurar componente de áudio
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->bAutoActivate = false;
        DialogueAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
    }
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Sistema iniciado"));
}

void ANarrativeDialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!CachedPlayer)
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        return;
    }
    
    // Update de diálogos activos
    UpdateActiveDialogues(DeltaTime);
    
    // Update de timers
    ProximityCheckTimer += DeltaTime;
    ContextCheckTimer += DeltaTime;
    
    // Verificações automáticas
    if (bAutoTriggerDialogues)
    {
        if (ProximityCheckTimer >= ProximityCheckInterval)
        {
            CheckPlayerProximity();
            ProximityCheckTimer = 0.0f;
        }
        
        if (ContextCheckTimer >= ContextCheckInterval)
        {
            CheckTriggerConditions();
            ContextCheckTimer = 0.0f;
        }
    }
    
    // Processar queue de diálogos
    ProcessDialogueQueue();
    
    // Update de cooldowns
    for (auto& CooldownPair : DialogueCooldowns)
    {
        CooldownPair.Value -= DeltaTime;
    }
}

bool ANarrativeDialogueManager::PlayDialogue(const FString& DialogueID)
{
    if (DialogueID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: DialogueID vazio"));
        return false;
    }
    
    FNarr_DialogueEntry* DialogueData = FindDialogueByID(DialogueID);
    if (!DialogueData)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Diálogo não encontrado: %s"), *DialogueID);
        return false;
    }
    
    if (!CanPlayDialogue(*DialogueData))
    {
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Diálogo não disponível: %s"), *DialogueID);
        return false;
    }
    
    StartDialoguePlayback(*DialogueData);
    return true;
}

void ANarrativeDialogueManager::StopCurrentDialogue()
{
    if (bIsDialoguePlaying && DialogueAudioComponent)
    {
        DialogueAudioComponent->Stop();
        bIsDialoguePlaying = false;
        
        OnDialogueInterrupted(CurrentDialogue.DialogueData);
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Diálogo interrompido"));
    }
}

void ANarrativeDialogueManager::TriggerDialogueByCondition(ENarr_TriggerCondition Condition, const FVector& Location)
{
    TArray<FNarr_DialogueEntry> TriggeredDialogues = GetTriggeredDialogues(Condition, Location);
    
    for (const FNarr_DialogueEntry& DialogueEntry : TriggeredDialogues)
    {
        if (CanPlayDialogue(DialogueEntry))
        {
            DialogueQueue.Add(DialogueEntry);
            UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Diálogo adicionado à queue: %s"), *DialogueEntry.DialogueID);
        }
    }
}

void ANarrativeDialogueManager::RegisterDialogueCompletion(const FString& DialogueID)
{
    if (!DialogueID.IsEmpty() && !PlayedDialogueIDs.Contains(DialogueID))
    {
        PlayedDialogueIDs.Add(DialogueID);
        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Diálogo registado como completo: %s"), *DialogueID);
    }
}

bool ANarrativeDialogueManager::IsDialogueAvailable(const FString& DialogueID) const
{
    const FNarr_DialogueEntry* DialogueData = const_cast<ANarrativeDialogueManager*>(this)->FindDialogueByID(DialogueID);
    return DialogueData ? CanPlayDialogue(*DialogueData) : false;
}

TArray<FNarr_DialogueEntry> ANarrativeDialogueManager::GetDialoguesByType(ENarr_DialogueType DialogueType) const
{
    TArray<FNarr_DialogueEntry> FilteredDialogues;
    
    if (!DialogueDataTable)
    {
        return FilteredDialogues;
    }
    
    TArray<FNarr_DialogueEntry*> AllDialogues;
    DialogueDataTable->GetAllRows<FNarr_DialogueEntry>(TEXT("GetDialoguesByType"), AllDialogues);
    
    for (const FNarr_DialogueEntry* DialogueEntry : AllDialogues)
    {
        if (DialogueEntry && DialogueEntry->DialogueType == DialogueType)
        {
            FilteredDialogues.Add(*DialogueEntry);
        }
    }
    
    return FilteredDialogues;
}

float ANarrativeDialogueManager::GetDialogueProgress() const
{
    if (bIsDialoguePlaying && CurrentDialogue.DialogueData.Duration > 0.0f)
    {
        float ElapsedTime = CurrentDialogue.DialogueData.Duration - CurrentDialogue.TimeRemaining;
        return FMath::Clamp(ElapsedTime / CurrentDialogue.DialogueData.Duration, 0.0f, 1.0f);
    }
    
    return 0.0f;
}

void ANarrativeDialogueManager::UpdateActiveDialogues(float DeltaTime)
{
    if (bIsDialoguePlaying)
    {
        CurrentDialogue.TimeRemaining -= DeltaTime;
        
        if (CurrentDialogue.TimeRemaining <= 0.0f)
        {
            CompleteDialogue();
        }
    }
}

void ANarrativeDialogueManager::CheckTriggerConditions()
{
    CheckDinosaurSightings();
    CheckResourceDiscoveries();
    CheckDangerSituations();
}

void ANarrativeDialogueManager::ProcessDialogueQueue()
{
    if (bIsDialoguePlaying || DialogueQueue.Num() == 0)
    {
        return;
    }
    
    // Ordenar por prioridade
    DialogueQueue.Sort([](const FNarr_DialogueEntry& A, const FNarr_DialogueEntry& B) {
        return A.Priority > B.Priority;
    });
    
    // Reproduzir o diálogo de maior prioridade
    FNarr_DialogueEntry NextDialogue = DialogueQueue[0];
    DialogueQueue.RemoveAt(0);
    
    StartDialoguePlayback(NextDialogue);
}

FNarr_DialogueEntry* ANarrativeDialogueManager::FindDialogueByID(const FString& DialogueID)
{
    if (!DialogueDataTable || DialogueID.IsEmpty())
    {
        return nullptr;
    }
    
    return DialogueDataTable->FindRow<FNarr_DialogueEntry>(FName(*DialogueID), TEXT("FindDialogueByID"));
}

TArray<FNarr_DialogueEntry> ANarrativeDialogueManager::GetTriggeredDialogues(ENarr_TriggerCondition Condition, const FVector& Location)
{
    TArray<FNarr_DialogueEntry> TriggeredDialogues;
    
    if (!DialogueDataTable)
    {
        return TriggeredDialogues;
    }
    
    TArray<FNarr_DialogueEntry*> AllDialogues;
    DialogueDataTable->GetAllRows<FNarr_DialogueEntry>(TEXT("GetTriggeredDialogues"), AllDialogues);
    
    for (const FNarr_DialogueEntry* DialogueEntry : AllDialogues)
    {
        if (DialogueEntry && DialogueEntry->TriggerCondition == Condition)
        {
            // Verificar distância se relevante
            if (Location != FVector::ZeroVector && DialogueEntry->TriggerRadius > 0.0f)
            {
                float Distance = FVector::Dist(Location, DialogueEntry->TriggerLocation);
                if (Distance <= DialogueEntry->TriggerRadius)
                {
                    TriggeredDialogues.Add(*DialogueEntry);
                }
            }
            else
            {
                TriggeredDialogues.Add(*DialogueEntry);
            }
        }
    }
    
    return TriggeredDialogues;
}

bool ANarrativeDialogueManager::CanPlayDialogue(const FNarr_DialogueEntry& DialogueData) const
{
    // Verificar se já foi reproduzido e não pode repetir
    if (!DialogueData.bCanRepeat && PlayedDialogueIDs.Contains(DialogueData.DialogueID))
    {
        return false;
    }
    
    // Verificar cooldown
    if (DialogueCooldowns.Contains(DialogueData.DialogueID))
    {
        float RemainingCooldown = DialogueCooldowns[DialogueData.DialogueID];
        if (RemainingCooldown > 0.0f)
        {
            return false;
        }
    }
    
    // Verificar se há espaço para mais diálogos
    if (bIsDialoguePlaying && MaxConcurrentDialogues <= 1)
    {
        return false;
    }
    
    return true;
}

void ANarrativeDialogueManager::StartDialoguePlayback(const FNarr_DialogueEntry& DialogueData)
{
    if (!DialogueAudioComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("NarrativeDialogueManager: AudioComponent não encontrado"));
        return;
    }
    
    // Parar diálogo actual se necessário
    if (bIsDialoguePlaying)
    {
        StopCurrentDialogue();
    }
    
    // Configurar novo diálogo
    CurrentDialogue.DialogueData = DialogueData;
    CurrentDialogue.TimeRemaining = DialogueData.Duration;
    CurrentDialogue.bIsPlaying = true;
    CurrentDialogue.LastPlayedTime = GetWorld()->GetTimeSeconds();
    
    bIsDialoguePlaying = true;
    
    // Reproduzir áudio se disponível
    if (DialogueData.AudioClip.IsValid())
    {
        USoundCue* SoundCue = DialogueData.AudioClip.LoadSynchronous();
        if (SoundCue)
        {
            DialogueAudioComponent->SetSound(SoundCue);
            DialogueAudioComponent->Play();
        }
    }
    
    // Definir cooldown
    DialogueCooldowns.Add(DialogueData.DialogueID, DialogueData.CooldownTime);
    
    // Evento de início
    OnDialogueStarted(DialogueData);
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Diálogo iniciado: %s"), *DialogueData.DialogueID);
}

void ANarrativeDialogueManager::CompleteDialogue()
{
    if (!bIsDialoguePlaying)
    {
        return;
    }
    
    // Registar conclusão
    RegisterDialogueCompletion(CurrentDialogue.DialogueData.DialogueID);
    
    // Evento de conclusão
    OnDialogueCompleted(CurrentDialogue.DialogueData);
    
    // Reset do estado
    bIsDialoguePlaying = false;
    CurrentDialogue = FNarr_ActiveDialogue();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Diálogo concluído"));
}

void ANarrativeDialogueManager::CheckPlayerProximity()
{
    if (!CachedPlayer)
    {
        return;
    }
    
    FVector PlayerLocation = CachedPlayer->GetActorLocation();
    TriggerDialogueByCondition(ENarr_TriggerCondition::PlayerProximity, PlayerLocation);
}

void ANarrativeDialogueManager::CheckDinosaurSightings()
{
    // Implementação básica - pode ser expandida com sistema de visão
    if (CachedPlayer)
    {
        FVector PlayerLocation = CachedPlayer->GetActorLocation();
        TriggerDialogueByCondition(ENarr_TriggerCondition::DinosaurSighting, PlayerLocation);
    }
}

void ANarrativeDialogueManager::CheckResourceDiscoveries()
{
    // Implementação básica - pode ser expandida com sistema de recursos
    if (CachedPlayer)
    {
        FVector PlayerLocation = CachedPlayer->GetActorLocation();
        TriggerDialogueByCondition(ENarr_TriggerCondition::ResourceDiscovery, PlayerLocation);
    }
}

void ANarrativeDialogueManager::CheckDangerSituations()
{
    // Implementação básica - pode ser expandida com sistema de perigo
    if (CachedPlayer)
    {
        FVector PlayerLocation = CachedPlayer->GetActorLocation();
        TriggerDialogueByCondition(ENarr_TriggerCondition::DangerDetected, PlayerLocation);
    }
}

APawn* ANarrativeDialogueManager::GetPlayerPawn() const
{
    return CachedPlayer;
}

float ANarrativeDialogueManager::GetDistanceToPlayer(const FVector& Location) const
{
    if (!CachedPlayer)
    {
        return MAX_FLT;
    }
    
    return FVector::Dist(CachedPlayer->GetActorLocation(), Location);
}

bool ANarrativeDialogueManager::IsLocationVisible(const FVector& Location) const
{
    if (!CachedPlayer)
    {
        return false;
    }
    
    FVector PlayerLocation = CachedPlayer->GetActorLocation();
    FHitResult HitResult;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        PlayerLocation,
        Location,
        ECC_Visibility
    );
    
    return !bHit;
}