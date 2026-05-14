#include "Narr_DialogueSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    bIsDialogueActive = false;
    CurrentNPC = nullptr;
    CurrentDialogueIndex = 0;
    
    // Create audio component for dialogue playback
    DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
    if (DialogueAudioComponent)
    {
        DialogueAudioComponent->bAutoActivate = false;
        DialogueAudioComponent->SetVolumeMultiplier(1.0f);
    }
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultNPCs();
    InitializeStoryEvents();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Dialogue System initialized with %d NPCs and %d story events"), 
           RegisteredNPCs.Num(), StoryEvents.Num());
}

void UNarr_DialogueSystem::InitializeDefaultNPCs()
{
    // Tribal Elder - Wisdom and guidance
    FNarr_NPCProfile TribalElder;
    TribalElder.NPCName = TEXT("TribalElder");
    TribalElder.Role = ENarr_NPCRole::TribalElder;
    TribalElder.BackgroundStory = FText::FromString(TEXT("The eldest member of the tribe, keeper of ancient knowledge and survival wisdom. Has witnessed many migrations and knows the patterns of the great beasts."));
    TribalElder.TrustLevel = 0.8f;
    TribalElder.bIsHostile = false;
    TribalElder.TerritoryCenter = FVector(-1000, 2000, 150);
    TribalElder.TerritoryRadius = 500.0f;
    
    // Add dialogue lines for Tribal Elder
    FNarr_DialogueLine ElderGreeting;
    ElderGreeting.DialogueText = FText::FromString(TEXT("Welcome, young one. The spirits of the ancestors guide your steps to our fire."));
    ElderGreeting.DialogueType = ENarr_DialogueType::Greeting;
    ElderGreeting.Duration = 6.0f;
    TribalElder.DialogueLines.Add(ElderGreeting);
    
    FNarr_DialogueLine ElderWisdom;
    ElderWisdom.DialogueText = FText::FromString(TEXT("The great migration has begun. Our scouts report massive herds moving through the eastern valleys."));
    ElderWisdom.DialogueType = ENarr_DialogueType::Storytelling;
    ElderWisdom.Duration = 8.0f;
    TribalElder.DialogueLines.Add(ElderWisdom);
    
    RegisteredNPCs.Add(TribalElder);
    
    // Hunt Master - Combat and hunting expertise
    FNarr_NPCProfile HuntMaster;
    HuntMaster.NPCName = TEXT("HuntMaster");
    HuntMaster.Role = ENarr_NPCRole::HuntMaster;
    HuntMaster.BackgroundStory = FText::FromString(TEXT("Master hunter and tracker, skilled in the ways of predator and prey. Leads hunting parties against dangerous dinosaurs."));
    HuntMaster.TrustLevel = 0.6f;
    HuntMaster.bIsHostile = false;
    HuntMaster.TerritoryCenter = FVector(2000, -1000, 200);
    HuntMaster.TerritoryRadius = 800.0f;
    
    FNarr_DialogueLine HuntGreeting;
    HuntGreeting.DialogueText = FText::FromString(TEXT("Hunter, your timing is good. We have fresh tracks of a pack nearby."));
    HuntGreeting.DialogueType = ENarr_DialogueType::Greeting;
    HuntGreeting.Duration = 5.0f;
    HuntMaster.DialogueLines.Add(HuntGreeting);
    
    FNarr_DialogueLine HuntWarning;
    HuntWarning.DialogueText = FText::FromString(TEXT("Listen carefully, young hunter. The Velociraptors hunt in coordinated packs - they are cunning, patient, and deadly."));
    HuntWarning.DialogueType = ENarr_DialogueType::Warning;
    HuntWarning.Duration = 10.0f;
    HuntMaster.DialogueLines.Add(HuntWarning);
    
    RegisteredNPCs.Add(HuntMaster);
    
    // Water Seeker - Resource management and exploration
    FNarr_NPCProfile WaterSeeker;
    WaterSeeker.NPCName = TEXT("WaterSeeker");
    WaterSeeker.Role = ENarr_NPCRole::WaterSeeker;
    WaterSeeker.BackgroundStory = FText::FromString(TEXT("Expert in finding water sources and safe passage through dangerous territory. Essential for tribe survival."));
    WaterSeeker.TrustLevel = 0.7f;
    WaterSeeker.bIsHostile = false;
    WaterSeeker.TerritoryCenter = FVector(0, 3000, 100);
    WaterSeeker.TerritoryRadius = 600.0f;
    
    FNarr_DialogueLine WaterGreeting;
    WaterGreeting.DialogueText = FText::FromString(TEXT("Traveler, you look thirsty. I know where clean water flows, but the path is treacherous."));
    WaterGreeting.DialogueType = ENarr_DialogueType::Greeting;
    WaterGreeting.Duration = 7.0f;
    WaterSeeker.DialogueLines.Add(WaterGreeting);
    
    FNarr_DialogueLine WaterWarning;
    WaterWarning.DialogueText = FText::FromString(TEXT("The water runs red with blood. Something massive has fed upstream - we must find another source."));
    WaterWarning.DialogueType = ENarr_DialogueType::Warning;
    WaterWarning.Duration = 8.0f;
    WaterSeeker.DialogueLines.Add(WaterWarning);
    
    RegisteredNPCs.Add(WaterSeeker);
    
    // War Chief - Combat leadership and defense
    FNarr_NPCProfile WarChief;
    WarChief.NPCName = TEXT("WarChief");
    WarChief.Role = ENarr_NPCRole::WarChief;
    WarChief.BackgroundStory = FText::FromString(TEXT("Battle-hardened leader who has faced the greatest predators and lived. Commands respect through strength and tactical brilliance."));
    WarChief.TrustLevel = 0.5f;
    WarChief.bIsHostile = false;
    WarChief.TerritoryCenter = FVector(-2000, -3000, 180);
    WarChief.TerritoryRadius = 1000.0f;
    
    FNarr_DialogueLine ChiefGreeting;
    ChiefGreeting.DialogueText = FText::FromString(TEXT("Warrior, prove your worth or stay out of my way. This is no place for the weak."));
    ChiefGreeting.DialogueType = ENarr_DialogueType::Greeting;
    ChiefGreeting.Duration = 6.0f;
    WarChief.DialogueLines.Add(ChiefGreeting);
    
    FNarr_DialogueLine ChiefCombat;
    ChiefCombat.DialogueText = FText::FromString(TEXT("The bone-crusher approaches from the north ridge! All warriors to defensive positions!"));
    ChiefCombat.DialogueType = ENarr_DialogueType::Combat;
    ChiefCombat.Duration = 6.0f;
    WarChief.DialogueLines.Add(ChiefCombat);
    
    RegisteredNPCs.Add(WarChief);
}

void UNarr_DialogueSystem::InitializeStoryEvents()
{
    // The Great Migration Event
    FNarr_StoryEvent MigrationEvent;
    MigrationEvent.EventID = TEXT("GreatMigration");
    MigrationEvent.EventTitle = FText::FromString(TEXT("The Great Migration"));
    MigrationEvent.EventDescription = FText::FromString(TEXT("Massive herds of herbivorous dinosaurs begin their seasonal migration, creating opportunities and dangers for the tribe."));
    MigrationEvent.bIsTriggered = false;
    MigrationEvent.bIsCompleted = false;
    
    FNarr_DialogueLine MigrationDialogue;
    MigrationDialogue.DialogueText = FText::FromString(TEXT("The earth trembles beneath countless feet. The great herds move as one, following paths older than memory."));
    MigrationDialogue.DialogueType = ENarr_DialogueType::Storytelling;
    MigrationDialogue.Duration = 9.0f;
    MigrationEvent.EventDialogue.Add(MigrationDialogue);
    
    StoryEvents.Add(MigrationEvent);
    
    // Predator Territory Event
    FNarr_StoryEvent PredatorEvent;
    PredatorEvent.EventID = TEXT("PredatorTerritory");
    PredatorEvent.EventTitle = FText::FromString(TEXT("Predator Territory"));
    PredatorEvent.EventDescription = FText::FromString(TEXT("A powerful predator has claimed territory near the tribe's water source, forcing difficult decisions about survival."));
    PredatorEvent.bIsTriggered = false;
    PredatorEvent.bIsCompleted = false;
    
    FNarr_DialogueLine PredatorDialogue;
    PredatorDialogue.DialogueText = FText::FromString(TEXT("The thunder-foot has marked this land as its hunting ground. We must choose: fight, flee, or find another way."));
    PredatorDialogue.DialogueType = ENarr_DialogueType::Warning;
    PredatorDialogue.Duration = 8.0f;
    PredatorEvent.EventDialogue.Add(PredatorDialogue);
    
    StoryEvents.Add(PredatorEvent);
    
    // Tribal Alliance Event
    FNarr_StoryEvent AllianceEvent;
    AllianceEvent.EventID = TEXT("TribalAlliance");
    AllianceEvent.EventTitle = FText::FromString(TEXT("Tribal Alliance"));
    AllianceEvent.EventDescription = FText::FromString(TEXT("Another tribe seeks alliance against common threats, but trust must be earned through shared trials."));
    AllianceEvent.bIsTriggered = false;
    AllianceEvent.bIsCompleted = false;
    AllianceEvent.PrerequisiteEvents.Add(TEXT("GreatMigration"));
    
    FNarr_DialogueLine AllianceDialogue;
    AllianceDialogue.DialogueText = FText::FromString(TEXT("Strangers approach under the banner of peace, but in these lands, survival often requires difficult alliances."));
    AllianceDialogue.DialogueType = ENarr_DialogueType::QuestGiver;
    AllianceDialogue.Duration = 10.0f;
    AllianceEvent.EventDialogue.Add(AllianceDialogue);
    
    StoryEvents.Add(AllianceEvent);
}

void UNarr_DialogueSystem::RegisterNPC(const FNarr_NPCProfile& NPCProfile)
{
    // Check if NPC already exists
    for (int32 i = 0; i < RegisteredNPCs.Num(); i++)
    {
        if (RegisteredNPCs[i].NPCName == NPCProfile.NPCName)
        {
            RegisteredNPCs[i] = NPCProfile; // Update existing
            UE_LOG(LogTemp, Log, TEXT("Updated NPC profile: %s"), *NPCProfile.NPCName.ToString());
            return;
        }
    }
    
    // Add new NPC
    RegisteredNPCs.Add(NPCProfile);
    UE_LOG(LogTemp, Log, TEXT("Registered new NPC: %s"), *NPCProfile.NPCName.ToString());
}

FNarr_NPCProfile* UNarr_DialogueSystem::FindNPCByName(const FName& NPCName)
{
    for (int32 i = 0; i < RegisteredNPCs.Num(); i++)
    {
        if (RegisteredNPCs[i].NPCName == NPCName)
        {
            return &RegisteredNPCs[i];
        }
    }
    return nullptr;
}

TArray<FNarr_NPCProfile*> UNarr_DialogueSystem::GetNPCsInRadius(const FVector& Location, float Radius)
{
    TArray<FNarr_NPCProfile*> NPCsInRange;
    
    for (int32 i = 0; i < RegisteredNPCs.Num(); i++)
    {
        float Distance = FVector::Dist(RegisteredNPCs[i].TerritoryCenter, Location);
        if (Distance <= Radius + RegisteredNPCs[i].TerritoryRadius)
        {
            NPCsInRange.Add(&RegisteredNPCs[i]);
        }
    }
    
    return NPCsInRange;
}

bool UNarr_DialogueSystem::StartDialogue(const FName& NPCName, ENarr_DialogueType DialogueType)
{
    if (bIsDialogueActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue already active, ending current dialogue first"));
        EndDialogue();
    }
    
    FNarr_NPCProfile* NPC = FindNPCByName(NPCName);
    if (!NPC)
    {
        UE_LOG(LogTemp, Error, TEXT("NPC not found: %s"), *NPCName.ToString());
        return false;
    }
    
    // Find appropriate dialogue line
    for (int32 i = 0; i < NPC->DialogueLines.Num(); i++)
    {
        if (NPC->DialogueLines[i].DialogueType == DialogueType)
        {
            CurrentNPC = NPC;
            CurrentDialogueIndex = i;
            bIsDialogueActive = true;
            
            PlayDialogueAudio(NPC->DialogueLines[i]);
            OnDialogueStarted.Broadcast(NPCName, DialogueType);
            
            UE_LOG(LogTemp, Log, TEXT("Started dialogue with %s: %s"), 
                   *NPCName.ToString(), 
                   *NPC->DialogueLines[i].DialogueText.ToString());
            
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("No dialogue found for NPC %s with type %d"), 
           *NPCName.ToString(), (int32)DialogueType);
    return false;
}

void UNarr_DialogueSystem::EndDialogue()
{
    if (bIsDialogueActive && CurrentNPC)
    {
        OnDialogueEnded.Broadcast(CurrentNPC->NPCName);
        
        if (DialogueAudioComponent && DialogueAudioComponent->IsPlaying())
        {
            DialogueAudioComponent->Stop();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Ended dialogue with %s"), *CurrentNPC->NPCName.ToString());
    }
    
    bIsDialogueActive = false;
    CurrentNPC = nullptr;
    CurrentDialogueIndex = 0;
}

bool UNarr_DialogueSystem::PlayNextDialogueLine()
{
    if (!bIsDialogueActive || !CurrentNPC)
    {
        return false;
    }
    
    CurrentDialogueIndex++;
    if (CurrentDialogueIndex >= CurrentNPC->DialogueLines.Num())
    {
        EndDialogue();
        return false;
    }
    
    PlayDialogueAudio(CurrentNPC->DialogueLines[CurrentDialogueIndex]);
    return true;
}

void UNarr_DialogueSystem::SelectPlayerResponse(int32 ResponseIndex)
{
    if (!bIsDialogueActive || !CurrentNPC)
    {
        return;
    }
    
    const FNarr_DialogueLine& CurrentLine = CurrentNPC->DialogueLines[CurrentDialogueIndex];
    if (ResponseIndex >= 0 && ResponseIndex < CurrentLine.PlayerResponseOptions.Num())
    {
        UE_LOG(LogTemp, Log, TEXT("Player selected response: %s"), 
               *CurrentLine.PlayerResponseOptions[ResponseIndex].ToString());
        
        // Advance dialogue or trigger consequences based on response
        PlayNextDialogueLine();
    }
}

void UNarr_DialogueSystem::TriggerStoryEvent(const FName& EventID)
{
    for (int32 i = 0; i < StoryEvents.Num(); i++)
    {
        if (StoryEvents[i].EventID == EventID && !StoryEvents[i].bIsTriggered)
        {
            if (CanTriggerStoryEvent(StoryEvents[i]))
            {
                StoryEvents[i].bIsTriggered = true;
                OnStoryEventTriggered.Broadcast(EventID, StoryEvents[i].EventTitle);
                
                UE_LOG(LogTemp, Log, TEXT("Triggered story event: %s"), *EventID.ToString());
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Cannot trigger story event %s - prerequisites not met"), 
                       *EventID.ToString());
            }
        }
    }
}

bool UNarr_DialogueSystem::IsStoryEventCompleted(const FName& EventID)
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event.bIsCompleted;
        }
    }
    return false;
}

void UNarr_DialogueSystem::CompleteStoryEvent(const FName& EventID)
{
    for (int32 i = 0; i < StoryEvents.Num(); i++)
    {
        if (StoryEvents[i].EventID == EventID)
        {
            StoryEvents[i].bIsCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("Completed story event: %s"), *EventID.ToString());
            return;
        }
    }
}

void UNarr_DialogueSystem::UpdateNPCTrustLevel(const FName& NPCName, float TrustDelta)
{
    FNarr_NPCProfile* NPC = FindNPCByName(NPCName);
    if (NPC)
    {
        NPC->TrustLevel = FMath::Clamp(NPC->TrustLevel + TrustDelta, 0.0f, 1.0f);
        UE_LOG(LogTemp, Log, TEXT("Updated trust level for %s: %.2f"), 
               *NPCName.ToString(), NPC->TrustLevel);
    }
}

float UNarr_DialogueSystem::GetNPCTrustLevel(const FName& NPCName)
{
    FNarr_NPCProfile* NPC = FindNPCByName(NPCName);
    return NPC ? NPC->TrustLevel : 0.0f;
}

TArray<FNarr_DialogueLine> UNarr_DialogueSystem::GetDialogueByType(const FName& NPCName, ENarr_DialogueType DialogueType)
{
    TArray<FNarr_DialogueLine> FilteredDialogue;
    
    FNarr_NPCProfile* NPC = FindNPCByName(NPCName);
    if (NPC)
    {
        for (const FNarr_DialogueLine& Line : NPC->DialogueLines)
        {
            if (Line.DialogueType == DialogueType)
            {
                FilteredDialogue.Add(Line);
            }
        }
    }
    
    return FilteredDialogue;
}

bool UNarr_DialogueSystem::CanTriggerStoryEvent(const FNarr_StoryEvent& Event)
{
    // Check if all prerequisite events are completed
    for (const FName& PrereqID : Event.PrerequisiteEvents)
    {
        if (!IsStoryEventCompleted(PrereqID))
        {
            return false;
        }
    }
    return true;
}

void UNarr_DialogueSystem::PlayDialogueAudio(const FNarr_DialogueLine& DialogueLine)
{
    if (DialogueAudioComponent && DialogueLine.VoiceClip.IsValid())
    {
        USoundWave* SoundWave = DialogueLine.VoiceClip.LoadSynchronous();
        if (SoundWave)
        {
            DialogueAudioComponent->SetSound(SoundWave);
            DialogueAudioComponent->Play();
            
            UE_LOG(LogTemp, Log, TEXT("Playing dialogue audio for: %s"), 
                   *DialogueLine.DialogueText.ToString());
        }
    }
}