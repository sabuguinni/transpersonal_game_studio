#include "NarrativeDialogueSystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UNarrativeDialogueSystem::UNarrativeDialogueSystem()
{
    CurrentAudioComponent = nullptr;
    bDialoguePlaying = false;
}

void UNarrativeDialogueSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDialogueLibraries();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeDialogueSystem initialized"));
}

void UNarrativeDialogueSystem::InitializeDialogueLibraries()
{
    // Quest Dialogue Library
    FNarr_DialogueLine HuntQuest;
    HuntQuest.DialogueText = TEXT("Hunt the great beast in the southern plains. Bring back its hide to prove your worth as a hunter.");
    HuntQuest.DialogueType = ENarr_DialogueType::QuestGiver;
    HuntQuest.Duration = 8.0f;
    QuestDialogueLibrary.Add(TEXT("HuntTRex"), HuntQuest);

    FNarr_DialogueLine CaveQuest;
    CaveQuest.DialogueText = TEXT("The sacred cave holds ancient tools of our ancestors. Venture deep and gather the stone implements within.");
    CaveQuest.DialogueType = ENarr_DialogueType::QuestGiver;
    CaveQuest.Duration = 7.0f;
    QuestDialogueLibrary.Add(TEXT("ExploreCave"), CaveQuest);

    FNarr_DialogueLine GatherQuest;
    GatherQuest.DialogueText = TEXT("The tribe needs food and materials. Gather berries from the forest and stones from the riverbank.");
    GatherQuest.DialogueType = ENarr_DialogueType::QuestGiver;
    GatherQuest.Duration = 6.0f;
    QuestDialogueLibrary.Add(TEXT("GatherResources"), GatherQuest);

    // Warning Dialogue Library
    FNarr_DialogueLine TRexWarning;
    TRexWarning.DialogueText = TEXT("Warning! A massive predator stalks these lands. Stay alert, primitive hunter. The thunder of its footsteps echoes through the valley.");
    TRexWarning.DialogueType = ENarr_DialogueType::Warning;
    TRexWarning.Duration = 9.0f;
    WarningDialogueLibrary.Add(TEXT("TRex"), TRexWarning);

    FNarr_DialogueLine RaptorWarning;
    RaptorWarning.DialogueText = TEXT("Pack hunters approach! Velociraptors move like shadows through the tall grass. Keep your spear ready and watch your back.");
    RaptorWarning.DialogueType = ENarr_DialogueType::Warning;
    RaptorWarning.Duration = 8.0f;
    WarningDialogueLibrary.Add(TEXT("Velociraptor"), RaptorWarning);

    FNarr_DialogueLine StormWarning;
    StormWarning.DialogueText = TEXT("Dark clouds gather on the horizon. Seek shelter quickly before the storm unleashes its fury upon the land.");
    StormWarning.DialogueType = ENarr_DialogueType::Warning;
    StormWarning.Duration = 7.0f;
    WarningDialogueLibrary.Add(TEXT("Storm"), StormWarning);

    // Discovery Dialogue Library
    FNarr_DialogueLine FireDiscovery;
    FireDiscovery.DialogueText = TEXT("You have discovered the sacred flame! This ancient knowledge will keep you warm and cook your food. Guard it well.");
    FireDiscovery.DialogueType = ENarr_DialogueType::Discovery;
    FireDiscovery.Duration = 8.0f;
    DiscoveryDialogueLibrary.Add(TEXT("Fire"), FireDiscovery);

    FNarr_DialogueLine CaveDiscovery;
    CaveDiscovery.DialogueText = TEXT("A hidden cave reveals itself! Ancient paintings cover the walls, telling stories of those who came before.");
    CaveDiscovery.DialogueType = ENarr_DialogueType::Discovery;
    CaveDiscovery.Duration = 7.0f;
    DiscoveryDialogueLibrary.Add(TEXT("Cave"), CaveDiscovery);

    FNarr_DialogueLine ToolDiscovery;
    ToolDiscovery.DialogueText = TEXT("Sharp stone and sturdy wood combine to create a powerful tool. With this spear, you can hunt and defend yourself.");
    ToolDiscovery.DialogueType = ENarr_DialogueType::Discovery;
    ToolDiscovery.Duration = 7.0f;
    DiscoveryDialogueLibrary.Add(TEXT("Tool"), ToolDiscovery);
}

void UNarrativeDialogueSystem::PlayDialogue(const FString& NPCName, ENarr_DialogueType DialogueType)
{
    if (bDialoguePlaying)
    {
        StopCurrentDialogue();
    }

    if (RegisteredNPCs.Contains(NPCName))
    {
        const FNarr_NPCProfile& NPCProfile = RegisteredNPCs[NPCName];
        
        for (const FNarr_DialogueLine& Line : NPCProfile.DialogueLines)
        {
            if (Line.DialogueType == DialogueType)
            {
                PlayAudioDialogue(Line);
                UE_LOG(LogTemp, Warning, TEXT("Playing dialogue for %s: %s"), *NPCName, *Line.DialogueText);
                return;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No dialogue found for NPC: %s, Type: %d"), *NPCName, (int32)DialogueType);
}

void UNarrativeDialogueSystem::RegisterNPC(const FNarr_NPCProfile& NPCProfile)
{
    RegisteredNPCs.Add(NPCProfile.NPCName, NPCProfile);
    UE_LOG(LogTemp, Warning, TEXT("Registered NPC: %s with %d dialogue lines"), *NPCProfile.NPCName, NPCProfile.DialogueLines.Num());
}

void UNarrativeDialogueSystem::TriggerQuestDialogue(const FString& QuestID, const FString& NPCName)
{
    if (QuestDialogueLibrary.Contains(QuestID))
    {
        const FNarr_DialogueLine& QuestLine = QuestDialogueLibrary[QuestID];
        PlayAudioDialogue(QuestLine);
        UE_LOG(LogTemp, Warning, TEXT("Quest dialogue triggered - %s: %s"), *QuestID, *QuestLine.DialogueText);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest dialogue not found for ID: %s"), *QuestID);
    }
}

void UNarrativeDialogueSystem::PlayWarningDialogue(const FString& ThreatType, const FVector& Location)
{
    if (WarningDialogueLibrary.Contains(ThreatType))
    {
        const FNarr_DialogueLine& WarningLine = WarningDialogueLibrary[ThreatType];
        PlayAudioDialogue(WarningLine);
        UE_LOG(LogTemp, Warning, TEXT("Warning dialogue triggered - %s at location: %s"), *ThreatType, *Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Warning dialogue not found for threat: %s"), *ThreatType);
    }
}

void UNarrativeDialogueSystem::PlayDiscoveryDialogue(const FString& DiscoveryType)
{
    if (DiscoveryDialogueLibrary.Contains(DiscoveryType))
    {
        const FNarr_DialogueLine& DiscoveryLine = DiscoveryDialogueLibrary[DiscoveryType];
        PlayAudioDialogue(DiscoveryLine);
        UE_LOG(LogTemp, Warning, TEXT("Discovery dialogue triggered - %s: %s"), *DiscoveryType, *DiscoveryLine.DialogueText);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Discovery dialogue not found for type: %s"), *DiscoveryType);
    }
}

bool UNarrativeDialogueSystem::IsDialoguePlaying() const
{
    return bDialoguePlaying && CurrentAudioComponent && CurrentAudioComponent->IsPlaying();
}

void UNarrativeDialogueSystem::StopCurrentDialogue()
{
    if (CurrentAudioComponent && CurrentAudioComponent->IsPlaying())
    {
        CurrentAudioComponent->Stop();
    }
    bDialoguePlaying = false;
    UE_LOG(LogTemp, Warning, TEXT("Dialogue stopped"));
}

FNarr_DialogueLine UNarrativeDialogueSystem::GetDialogueForType(ENarr_DialogueType DialogueType, const FString& Context)
{
    FNarr_DialogueLine DefaultLine;
    DefaultLine.DialogueText = TEXT("...");
    DefaultLine.DialogueType = DialogueType;
    DefaultLine.Duration = 2.0f;

    switch (DialogueType)
    {
        case ENarr_DialogueType::Warning:
            if (WarningDialogueLibrary.Contains(Context))
            {
                return WarningDialogueLibrary[Context];
            }
            break;
        case ENarr_DialogueType::Discovery:
            if (DiscoveryDialogueLibrary.Contains(Context))
            {
                return DiscoveryDialogueLibrary[Context];
            }
            break;
        case ENarr_DialogueType::QuestGiver:
            if (QuestDialogueLibrary.Contains(Context))
            {
                return QuestDialogueLibrary[Context];
            }
            break;
        default:
            break;
    }

    return DefaultLine;
}

void UNarrativeDialogueSystem::PlayAudioDialogue(const FNarr_DialogueLine& DialogueLine)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No world context for audio playback"));
        return;
    }

    bDialoguePlaying = true;

    // For now, just log the dialogue text since we don't have audio files loaded
    UE_LOG(LogTemp, Warning, TEXT("DIALOGUE: %s"), *DialogueLine.DialogueText);

    // Set a timer to mark dialogue as finished
    FTimerHandle DialogueTimer;
    World->GetTimerManager().SetTimer(DialogueTimer, [this]()
    {
        bDialoguePlaying = false;
    }, DialogueLine.Duration, false);
}