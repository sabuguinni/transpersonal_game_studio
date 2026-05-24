#include "Narr_VoiceActedNPCSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TriggerBox.h"

ANarr_VoiceActedNPCSystem::ANarr_VoiceActedNPCSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize voice system
    bSystemActive = true;
    DialogueRange = 500.0f;
    VoiceVolume = 1.0f;
    CurrentDialogueIndex = 0;
    
    // Initialize NPC data
    InitializeNPCVoiceData();
}

void ANarr_VoiceActedNPCSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Voice Acted NPC System initialized"));
    
    // Find all NPCs in the level
    DiscoverNPCsInLevel();
    
    // Set up voice triggers
    SetupVoiceTriggers();
}

void ANarr_VoiceActedNPCSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bSystemActive) return;
    
    // Check for player proximity to NPCs
    CheckPlayerProximityToNPCs();
    
    // Update dialogue cooldowns
    UpdateDialogueCooldowns(DeltaTime);
}

void ANarr_VoiceActedNPCSystem::InitializeNPCVoiceData()
{
    // Kael the Hunter - Survival Expert
    FNarr_NPCVoiceData KaelData;
    KaelData.NPCName = TEXT("Kael");
    KaelData.CharacterType = ENarr_NPCType::SurvivalExpert;
    KaelData.VoiceAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778446248222_ThreatAnalyst.mp3");
    KaelData.DialogueLines.Add(TEXT("Greetings, survivor. I am Kael, master hunter of these dangerous lands."));
    KaelData.DialogueLines.Add(TEXT("The great beasts roam freely here, and only the prepared survive."));
    KaelData.DialogueLines.Add(TEXT("Watch the tracks, listen to the forest. The dinosaurs will teach you respect."));
    KaelData.LastDialogueTime = 0.0f;
    KaelData.DialogueCooldown = 10.0f;
    NPCVoiceDatabase.Add(TEXT("Kael_SurvivalExpert"), KaelData);
    
    // Nira the Gatherer - Resource Expert
    FNarr_NPCVoiceData NiraData;
    NiraData.NPCName = TEXT("Nira");
    NiraData.CharacterType = ENarr_NPCType::ResourceGatherer;
    NiraData.VoiceAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778446257968_ResourceScout.mp3");
    NiraData.DialogueLines.Add(TEXT("Welcome, young one. I am Nira, gatherer of the healing plants."));
    NiraData.DialogueLines.Add(TEXT("The forest provides all we need, but one must know where to look."));
    NiraData.DialogueLines.Add(TEXT("These berries are safe, but avoid the red ones near the cliffs."));
    NiraData.LastDialogueTime = 0.0f;
    NiraData.DialogueCooldown = 8.0f;
    NPCVoiceDatabase.Add(TEXT("Nira_ResourceGatherer"), NiraData);
    
    // Marcus the Threat Analyst
    FNarr_NPCVoiceData MarcusData;
    MarcusData.NPCName = TEXT("Marcus");
    MarcusData.CharacterType = ENarr_NPCType::ThreatAnalyst;
    MarcusData.VoiceAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778446248222_ThreatAnalyst.mp3");
    MarcusData.DialogueLines.Add(TEXT("Critical threat assessment! Massive Carnotaurus pack detected approaching."));
    MarcusData.DialogueLines.Add(TEXT("Stay alert, survivor. The predators are always watching."));
    MarcusData.DialogueLines.Add(TEXT("I've mapped the safe routes. Follow my markers to avoid the hunting grounds."));
    MarcusData.LastDialogueTime = 0.0f;
    MarcusData.DialogueCooldown = 15.0f;
    NPCVoiceDatabase.Add(TEXT("Marcus_ThreatAnalyst"), MarcusData);
    
    // Elena the Weather Observer
    FNarr_NPCVoiceData ElenaData;
    ElenaData.NPCName = TEXT("Elena");
    ElenaData.CharacterType = ENarr_NPCType::WeatherObserver;
    ElenaData.VoiceAudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1778446265914_WeatherObserver.mp3");
    ElenaData.DialogueLines.Add(TEXT("Weather observation alert! Massive storm system approaching."));
    ElenaData.DialogueLines.Add(TEXT("The winds carry news from distant lands. A storm comes tonight."));
    ElenaData.DialogueLines.Add(TEXT("When the pterosaurs fly low, seek shelter immediately."));
    ElenaData.LastDialogueTime = 0.0f;
    ElenaData.DialogueCooldown = 12.0f;
    NPCVoiceDatabase.Add(TEXT("Elena_WeatherObserver"), ElenaData);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d NPC voice profiles"), NPCVoiceDatabase.Num());
}

void ANarr_VoiceActedNPCSystem::DiscoverNPCsInLevel()
{
    if (!GetWorld()) return;
    
    TArray<AActor*> FoundNPCs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), FoundNPCs);
    
    for (AActor* Actor : FoundNPCs)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            FString ActorLabel = Character->GetActorLabel();
            
            // Check if this is one of our voice-acted NPCs
            if (NPCVoiceDatabase.Contains(ActorLabel))
            {
                ActiveNPCs.Add(Character);
                UE_LOG(LogTemp, Warning, TEXT("Registered voice NPC: %s"), *ActorLabel);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Discovered %d voice-acted NPCs in level"), ActiveNPCs.Num());
}

void ANarr_VoiceActedNPCSystem::SetupVoiceTriggers()
{
    if (!GetWorld()) return;
    
    TArray<AActor*> FoundTriggers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATriggerBox::StaticClass(), FoundTriggers);
    
    for (AActor* Actor : FoundTriggers)
    {
        if (ATriggerBox* TriggerBox = Cast<ATriggerBox>(Actor))
        {
            FString TriggerLabel = TriggerBox->GetActorLabel();
            
            // Check if this is a dialogue trigger
            if (TriggerLabel.Contains(TEXT("DialogueTrigger")))
            {
                // Bind overlap events
                TriggerBox->OnActorBeginOverlap.AddDynamic(this, &ANarr_VoiceActedNPCSystem::OnDialogueTriggerEnter);
                TriggerBox->OnActorEndOverlap.AddDynamic(this, &ANarr_VoiceActedNPCSystem::OnDialogueTriggerExit);
                
                DialogueTriggers.Add(TriggerBox);
                UE_LOG(LogTemp, Warning, TEXT("Setup dialogue trigger: %s"), *TriggerLabel);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Setup %d dialogue triggers"), DialogueTriggers.Num());
}

void ANarr_VoiceActedNPCSystem::CheckPlayerProximityToNPCs()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (ACharacter* NPC : ActiveNPCs)
    {
        if (!NPC) continue;
        
        float Distance = FVector::Dist(PlayerLocation, NPC->GetActorLocation());
        
        if (Distance <= DialogueRange)
        {
            FString NPCLabel = NPC->GetActorLabel();
            
            if (NPCVoiceDatabase.Contains(NPCLabel))
            {
                FNarr_NPCVoiceData& VoiceData = NPCVoiceDatabase[NPCLabel];
                
                // Check cooldown
                float CurrentTime = GetWorld()->GetTimeSeconds();
                if (CurrentTime - VoiceData.LastDialogueTime >= VoiceData.DialogueCooldown)
                {
                    TriggerNPCDialogue(NPCLabel);
                    VoiceData.LastDialogueTime = CurrentTime;
                }
            }
        }
    }
}

void ANarr_VoiceActedNPCSystem::UpdateDialogueCooldowns(float DeltaTime)
{
    // Cooldowns are handled in CheckPlayerProximityToNPCs using world time
    // This function can be used for additional timing logic if needed
}

void ANarr_VoiceActedNPCSystem::TriggerNPCDialogue(const FString& NPCName)
{
    if (!NPCVoiceDatabase.Contains(NPCName)) return;
    
    FNarr_NPCVoiceData& VoiceData = NPCVoiceDatabase[NPCName];
    
    if (VoiceData.DialogueLines.Num() == 0) return;
    
    // Get next dialogue line
    int32 LineIndex = FMath::RandRange(0, VoiceData.DialogueLines.Num() - 1);
    FString DialogueLine = VoiceData.DialogueLines[LineIndex];
    
    // Log the dialogue (in a real game, this would trigger audio playback)
    UE_LOG(LogTemp, Warning, TEXT("[%s]: %s"), *VoiceData.NPCName, *DialogueLine);
    
    // Broadcast dialogue event
    OnNPCDialogueTriggered.Broadcast(VoiceData.NPCName, DialogueLine, VoiceData.VoiceAudioURL);
    
    // Play audio if available (placeholder for actual audio system integration)
    PlayNPCVoiceLine(VoiceData);
}

void ANarr_VoiceActedNPCSystem::PlayNPCVoiceLine(const FNarr_NPCVoiceData& VoiceData)
{
    // In a full implementation, this would:
    // 1. Load the audio from the URL
    // 2. Create an audio component
    // 3. Play the sound at the NPC's location
    // 4. Handle 3D spatial audio
    
    UE_LOG(LogTemp, Warning, TEXT("Playing voice line for %s: %s"), *VoiceData.NPCName, *VoiceData.VoiceAudioURL);
    
    // For now, we'll just log the action
    // Future integration with UE5 audio system would go here
}

void ANarr_VoiceActedNPCSystem::OnDialogueTriggerEnter(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor) return;
    
    // Check if player entered trigger
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor == PlayerPawn)
    {
        // Find associated NPC and trigger dialogue
        FString TriggerName = OverlappedActor->GetActorLabel();
        
        if (TriggerName.Contains(TEXT("Kael")))
        {
            TriggerNPCDialogue(TEXT("Kael_SurvivalExpert"));
        }
        else if (TriggerName.Contains(TEXT("Nira")))
        {
            TriggerNPCDialogue(TEXT("Nira_ResourceGatherer"));
        }
        else if (TriggerName.Contains(TEXT("Marcus")))
        {
            TriggerNPCDialogue(TEXT("Marcus_ThreatAnalyst"));
        }
        else if (TriggerName.Contains(TEXT("Elena")))
        {
            TriggerNPCDialogue(TEXT("Elena_WeatherObserver"));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Player entered dialogue trigger: %s"), *TriggerName);
    }
}

void ANarr_VoiceActedNPCSystem::OnDialogueTriggerExit(AActor* OverlappedActor, AActor* OtherActor)
{
    if (!OtherActor) return;
    
    // Check if player exited trigger
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (OtherActor == PlayerPawn)
    {
        FString TriggerName = OverlappedActor->GetActorLabel();
        UE_LOG(LogTemp, Warning, TEXT("Player exited dialogue trigger: %s"), *TriggerName);
    }
}

void ANarr_VoiceActedNPCSystem::SetSystemActive(bool bActive)
{
    bSystemActive = bActive;
    UE_LOG(LogTemp, Warning, TEXT("Voice Acted NPC System %s"), bActive ? TEXT("ACTIVATED") : TEXT("DEACTIVATED"));
}

void ANarr_VoiceActedNPCSystem::SetDialogueRange(float NewRange)
{
    DialogueRange = FMath::Max(NewRange, 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("Dialogue range set to: %f"), DialogueRange);
}

void ANarr_VoiceActedNPCSystem::SetVoiceVolume(float NewVolume)
{
    VoiceVolume = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    UE_LOG(LogTemp, Warning, TEXT("Voice volume set to: %f"), VoiceVolume);
}

TArray<FString> ANarr_VoiceActedNPCSystem::GetActiveNPCNames() const
{
    TArray<FString> NPCNames;
    
    for (const auto& NPCPair : NPCVoiceDatabase)
    {
        NPCNames.Add(NPCPair.Value.NPCName);
    }
    
    return NPCNames;
}

FNarr_NPCVoiceData ANarr_VoiceActedNPCSystem::GetNPCVoiceData(const FString& NPCName) const
{
    if (NPCVoiceDatabase.Contains(NPCName))
    {
        return NPCVoiceDatabase[NPCName];
    }
    
    return FNarr_NPCVoiceData();
}