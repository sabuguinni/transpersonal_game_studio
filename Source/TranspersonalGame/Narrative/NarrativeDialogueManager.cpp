#include "NarrativeDialogueManager.h"
#include "TranspersonalCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componente raiz
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Criar componente de áudio para diálogos
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    DialogueAudioComponent->SetupAttachment(RootComponent);
    DialogueAudioComponent->bAutoActivate = false;

    // Criar componente de texto para legendas
    SubtitleText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SubtitleText"));
    SubtitleText->SetupAttachment(RootComponent);
    SubtitleText->SetRelativeLocation(FVector(0, 0, 100));
    SubtitleText->SetText(FText::FromString(TEXT("")));
    SubtitleText->SetTextRenderColor(FColor::White);
    SubtitleText->SetHorizontalAlignment(EHTA_Center);
    SubtitleText->SetWorldSize(24.0f);
    SubtitleText->SetVisibility(false);

    // Configurações padrão
    bIsPlayingDialogue = false;
    DialogueVolume = 1.0f;
    SubtitleDisplayTime = 3.0f;
    DialogueStartTime = 0.0f;
    PlayerCharacter = nullptr;
}

void ANarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar base de dados de diálogos
    InitializeDialogueDatabase();
    
    // Encontrar referência ao personagem do jogador
    PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Player character not found"));
    }
}

void ANarrativeDialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Actualizar display de legendas se necessário
    if (bIsPlayingDialogue)
    {
        UpdateSubtitleDisplay();
    }
}

void ANarrativeDialogueManager::InitializeDialogueDatabase()
{
    DialogueDatabase.Empty();
    CreateDefaultDialogueEntries();
    
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Dialogue database initialized with %d entries"), DialogueDatabase.Num());
}

void ANarrativeDialogueManager::CreateDefaultDialogueEntries()
{
    // Entrada 1: Aviso táctico sobre T-Rex
    FNarr_DialogueEntry TacticalWarning;
    TacticalWarning.DialogueID = TEXT("tactical_trex_warning");
    TacticalWarning.DialogueType = ENarr_DialogueType::TacticalWarning;
    TacticalWarning.TriggerType = ENarr_DialogueTrigger::DinosaurSighting;
    TacticalWarning.DialogueText = TEXT("Attention, survivor. You've entered a critical zone. The large predator ahead is a T-Rex - apex hunter of this territory. Stay low, move slowly, and avoid direct eye contact.");
    TacticalWarning.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777811654649_TacticalNarrator.mp3");
    TacticalWarning.CharacterName = TEXT("Tactical Narrator");
    TacticalWarning.Duration = 15.0f;
    TacticalWarning.Priority = 10;
    TacticalWarning.bIsRepeatable = true;
    DialogueDatabase.Add(TacticalWarning);

    // Entrada 2: Log de investigação de campo
    FNarr_DialogueEntry FieldResearch;
    FieldResearch.DialogueID = TEXT("field_research_raptors");
    FieldResearch.DialogueType = ENarr_DialogueType::FieldResearch;
    FieldResearch.TriggerType = ENarr_DialogueTrigger::PlayerProximity;
    FieldResearch.DialogueText = TEXT("Field research log, day 127. The pack dynamics I'm observing are extraordinary. These raptors communicate through complex vocalizations and coordinated hunting patterns.");
    FieldResearch.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777811657324_FieldResearcher.mp3");
    FieldResearch.CharacterName = TEXT("Field Researcher");
    FieldResearch.Duration = 16.0f;
    FieldResearch.Priority = 5;
    FieldResearch.bIsRepeatable = false;
    DialogueDatabase.Add(FieldResearch);

    // Entrada 3: Alerta de segurança sobre herbívoros
    FNarr_DialogueEntry SafetyAlert;
    SafetyAlert.DialogueID = TEXT("safety_herbivore_migration");
    SafetyAlert.DialogueType = ENarr_DialogueType::SafetyAlert;
    SafetyAlert.TriggerType = ENarr_DialogueTrigger::EnvironmentalChange;
    SafetyAlert.DialogueText = TEXT("Warning: Environmental hazard detected. Massive herbivore migration in progress. Triceratops herd moving through the valley - maintain safe distance.");
    SafetyAlert.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777811660004_SafetyNarrator.mp3");
    SafetyAlert.CharacterName = TEXT("Safety Narrator");
    SafetyAlert.Duration = 16.0f;
    SafetyAlert.Priority = 8;
    SafetyAlert.bIsRepeatable = true;
    DialogueDatabase.Add(SafetyAlert);

    // Entrada 4: Sabedoria tribal
    FNarr_DialogueEntry TribalWisdom;
    TribalWisdom.DialogueID = TEXT("tribal_survival_wisdom");
    TribalWisdom.DialogueType = ENarr_DialogueType::TribalWisdom;
    TribalWisdom.TriggerType = ENarr_DialogueTrigger::QuestProgress;
    TribalWisdom.DialogueText = TEXT("Listen carefully, newcomer. This land has rules written in blood and bone. The raptors hunt in packs at dawn and dusk. The T-Rex claims the eastern ridge as his domain.");
    TribalWisdom.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1777811662404_TribalElder.mp3");
    TribalWisdom.CharacterName = TEXT("Tribal Elder");
    TribalWisdom.Duration = 15.0f;
    TribalWisdom.Priority = 7;
    TribalWisdom.bIsRepeatable = false;
    DialogueDatabase.Add(TribalWisdom);
}

void ANarrativeDialogueManager::PlayDialogue(const FString& DialogueID)
{
    // Procurar diálogo na base de dados
    FNarr_DialogueEntry* FoundDialogue = DialogueDatabase.FindByPredicate([DialogueID](const FNarr_DialogueEntry& Entry)
    {
        return Entry.DialogueID == DialogueID;
    });

    if (FoundDialogue)
    {
        // Parar diálogo actual se estiver a tocar
        if (bIsPlayingDialogue)
        {
            StopCurrentDialogue();
        }

        CurrentDialogue = *FoundDialogue;
        bIsPlayingDialogue = true;
        DialogueStartTime = GetWorld()->GetTimeSeconds();

        // Mostrar legendas
        SubtitleText->SetText(FText::FromString(CurrentDialogue.DialogueText));
        SubtitleText->SetVisibility(true);

        // Configurar timer para esconder legendas
        GetWorld()->GetTimerManager().SetTimer(SubtitleTimer, this, &ANarrativeDialogueManager::OnDialogueFinished, CurrentDialogue.Duration, false);

        // Marcar como reproduzido se não for repetível
        if (!FoundDialogue->bIsRepeatable)
        {
            FoundDialogue->bHasBeenPlayed = true;
        }

        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Playing dialogue '%s' by %s"), *CurrentDialogue.DialogueID, *CurrentDialogue.CharacterName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueManager: Dialogue ID '%s' not found"), *DialogueID);
    }
}

void ANarrativeDialogueManager::PlayDialogueByType(ENarr_DialogueType DialogueType)
{
    // Encontrar diálogos do tipo especificado
    TArray<FNarr_DialogueEntry> MatchingDialogues = GetDialoguesByType(DialogueType);
    
    if (MatchingDialogues.Num() > 0)
    {
        // Escolher o de maior prioridade que ainda não foi reproduzido (se aplicável)
        FNarr_DialogueEntry* BestDialogue = nullptr;
        int32 HighestPriority = 0;

        for (FNarr_DialogueEntry& Dialogue : MatchingDialogues)
        {
            if (Dialogue.bIsRepeatable || !Dialogue.bHasBeenPlayed)
            {
                if (Dialogue.Priority > HighestPriority)
                {
                    BestDialogue = &Dialogue;
                    HighestPriority = Dialogue.Priority;
                }
            }
        }

        if (BestDialogue)
        {
            PlayDialogue(BestDialogue->DialogueID);
        }
    }
}

void ANarrativeDialogueManager::StopCurrentDialogue()
{
    if (bIsPlayingDialogue)
    {
        bIsPlayingDialogue = false;
        
        // Parar áudio se estiver a tocar
        if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
        {
            DialogueAudioComponent->Stop();
        }

        // Esconder legendas
        SubtitleText->SetVisibility(false);

        // Limpar timers
        GetWorld()->GetTimerManager().ClearTimer(SubtitleTimer);
        GetWorld()->GetTimerManager().ClearTimer(DialogueTimer);

        UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Dialogue stopped"));
    }
}

bool ANarrativeDialogueManager::TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FVector& TriggerLocation)
{
    if (bIsPlayingDialogue)
    {
        return false; // Já está a reproduzir um diálogo
    }

    FNarr_DialogueEntry BestDialogue = GetHighestPriorityDialogue(TriggerType);
    
    if (!BestDialogue.DialogueID.IsEmpty())
    {
        PlayDialogue(BestDialogue.DialogueID);
        return true;
    }

    return false;
}

void ANarrativeDialogueManager::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueDatabase.Add(NewEntry);
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Added new dialogue entry '%s'"), *NewEntry.DialogueID);
}

TArray<FNarr_DialogueEntry> ANarrativeDialogueManager::GetDialoguesByType(ENarr_DialogueType DialogueType)
{
    TArray<FNarr_DialogueEntry> MatchingDialogues;
    
    for (const FNarr_DialogueEntry& Dialogue : DialogueDatabase)
    {
        if (Dialogue.DialogueType == DialogueType)
        {
            MatchingDialogues.Add(Dialogue);
        }
    }

    return MatchingDialogues;
}

FNarr_DialogueEntry ANarrativeDialogueManager::GetHighestPriorityDialogue(ENarr_DialogueTrigger TriggerType)
{
    FNarr_DialogueEntry BestDialogue;
    int32 HighestPriority = 0;

    for (const FNarr_DialogueEntry& Dialogue : DialogueDatabase)
    {
        if (Dialogue.TriggerType == TriggerType)
        {
            if (Dialogue.bIsRepeatable || !Dialogue.bHasBeenPlayed)
            {
                if (Dialogue.Priority > HighestPriority)
                {
                    BestDialogue = Dialogue;
                    HighestPriority = Dialogue.Priority;
                }
            }
        }
    }

    return BestDialogue;
}

void ANarrativeDialogueManager::SetSubtitleVisibility(bool bVisible)
{
    SubtitleText->SetVisibility(bVisible);
}

bool ANarrativeDialogueManager::IsDialoguePlaying() const
{
    return bIsPlayingDialogue;
}

void ANarrativeDialogueManager::OnDialogueFinished()
{
    StopCurrentDialogue();
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Dialogue finished"));
}

void ANarrativeDialogueManager::UpdateSubtitleDisplay()
{
    if (bIsPlayingDialogue && PlayerCharacter)
    {
        // Posicionar legendas acima do jogador
        FVector PlayerLocation = PlayerCharacter->GetActorLocation();
        FVector SubtitleLocation = PlayerLocation + FVector(0, 0, 200);
        SubtitleText->SetWorldLocation(SubtitleLocation);

        // Rodar legendas para ficarem viradas para a câmara
        if (PlayerCharacter->GetController())
        {
            FRotator CameraRotation = PlayerCharacter->GetController()->GetControlRotation();
            FRotator SubtitleRotation = FRotator(0, CameraRotation.Yaw, 0);
            SubtitleText->SetWorldRotation(SubtitleRotation);
        }
    }
}

void ANarrativeDialogueManager::LoadAudioFromURL(const FString& AudioURL)
{
    // Implementação futura para carregar áudio de URL
    // Por agora, usar sistema de áudio local do UE5
    UE_LOG(LogTemp, Log, TEXT("NarrativeDialogueManager: Audio URL registered: %s"), *AudioURL);
}