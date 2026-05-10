#include "Narr_NPCInteractionSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UNarr_NPCInteractionSystem::UNarr_NPCInteractionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update every 0.5 seconds for performance

    // Default settings
    InteractionRange = 800.0f;
    MoodUpdateInterval = 2.0f;
    SocialShareRadius = 1500.0f;
    LastMoodUpdate = 0.0f;
    bInGroup = false;
    CurrentTrustLevel = 0.3f;
    CurrentMood = ENarr_NPCMoodState::Neutral;

    // Initialize default personality
    NPCPersonality.NPCName = TEXT("Survivor");
    NPCPersonality.NPCRole = TEXT("Field Researcher");
    NPCPersonality.DefaultMood = ENarr_NPCMoodState::Neutral;
    NPCPersonality.AggressionLevel = 0.3f;
    NPCPersonality.FearThreshold = 0.6f;
    NPCPersonality.TrustLevel = 0.3f;
}

void UNarr_NPCInteractionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentMood = NPCPersonality.DefaultMood;
    CurrentTrustLevel = NPCPersonality.TrustLevel;
    
    // Initialize with some basic dialogue options
    if (NPCPersonality.DialogueOptions.Num() == 0)
    {
        FNarr_NPCDialogueEntry CasualGreeting;
        CasualGreeting.DialogueText = TEXT("Stay alert out here. The dinosaurs are more active today.");
        CasualGreeting.InteractionType = ENarr_NPCInteractionType::Casual;
        CasualGreeting.RequiredMood = ENarr_NPCMoodState::Neutral;
        CasualGreeting.TriggerDistance = 600.0f;
        CasualGreeting.VoicelineID = TEXT("casual_greeting_01");
        NPCPersonality.DialogueOptions.Add(CasualGreeting);

        FNarr_NPCDialogueEntry WarningDialogue;
        WarningDialogue.DialogueText = TEXT("I've seen fresh Carnotaurus tracks to the east. Avoid that area.");
        WarningDialogue.InteractionType = ENarr_NPCInteractionType::Warning;
        WarningDialogue.RequiredMood = ENarr_NPCMoodState::Cautious;
        WarningDialogue.TriggerDistance = 800.0f;
        WarningDialogue.VoicelineID = TEXT("warning_carnotaurus");
        NPCPersonality.DialogueOptions.Add(WarningDialogue);

        FNarr_NPCDialogueEntry EmergencyDialogue;
        EmergencyDialogue.DialogueText = TEXT("Emergency! Massive predator approaching from the northwest!");
        EmergencyDialogue.InteractionType = ENarr_NPCInteractionType::Emergency;
        EmergencyDialogue.RequiredMood = ENarr_NPCMoodState::Panicked;
        EmergencyDialogue.TriggerDistance = 1200.0f;
        EmergencyDialogue.VoicelineID = TEXT("emergency_alert");
        NPCPersonality.DialogueOptions.Add(EmergencyDialogue);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC Interaction System initialized for %s"), *NPCPersonality.NPCName);
}

void UNarr_NPCInteractionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update mood periodically
    if (GetWorld()->GetTimeSeconds() - LastMoodUpdate > MoodUpdateInterval)
    {
        UpdateMoodBasedOnEvents(DeltaTime);
        UpdateNearbyNPCs();
        LastMoodUpdate = GetWorld()->GetTimeSeconds();
    }
}

void UNarr_NPCInteractionSystem::InitializeNPCPersonality(const FNarr_NPCPersonality& Personality)
{
    NPCPersonality = Personality;
    CurrentMood = Personality.DefaultMood;
    CurrentTrustLevel = Personality.TrustLevel;
    
    UE_LOG(LogTemp, Log, TEXT("NPC Personality initialized: %s (%s)"), *Personality.NPCName, *Personality.NPCRole);
}

bool UNarr_NPCInteractionSystem::CanInteractWithPlayer(ATranspersonalCharacter* Player)
{
    if (!Player)
    {
        return false;
    }

    // Check if player is in range
    if (!IsPlayerInRange(Player))
    {
        return false;
    }

    // Check mood restrictions
    if (CurrentMood == ENarr_NPCMoodState::Hostile || CurrentMood == ENarr_NPCMoodState::Panicked)
    {
        return false;
    }

    // Check trust level
    if (CurrentTrustLevel < 0.1f)
    {
        return false;
    }

    return true;
}

FNarr_NPCDialogueEntry UNarr_NPCInteractionSystem::GetBestDialogueOption(ATranspersonalCharacter* Player)
{
    FNarr_NPCDialogueEntry BestOption;
    
    if (!Player || NPCPersonality.DialogueOptions.Num() == 0)
    {
        return BestOption;
    }

    float BestScore = -1.0f;
    
    for (const FNarr_NPCDialogueEntry& Option : NPCPersonality.DialogueOptions)
    {
        float Score = 0.0f;
        
        // Check mood compatibility
        if (Option.RequiredMood == CurrentMood)
        {
            Score += 10.0f;
        }
        else if (Option.RequiredMood == ENarr_NPCMoodState::Neutral)
        {
            Score += 5.0f;
        }
        
        // Check distance
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
        if (Distance <= Option.TriggerDistance)
        {
            Score += (Option.TriggerDistance - Distance) / Option.TriggerDistance * 5.0f;
        }
        
        // Check line of sight if required
        if (Option.bRequiresLineOfSight && !HasLineOfSight(Player))
        {
            Score = 0.0f;
        }
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestOption = Option;
        }
    }
    
    return BestOption;
}

void UNarr_NPCInteractionSystem::TriggerInteraction(ATranspersonalCharacter* Player, ENarr_NPCInteractionType InteractionType)
{
    if (!Player)
    {
        return;
    }

    FNarr_NPCDialogueEntry DialogueOption = GetBestDialogueOption(Player);
    
    if (!DialogueOption.DialogueText.IsEmpty())
    {
        // Play the dialogue
        UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *NPCPersonality.NPCName, *DialogueOption.DialogueText);
        
        // Play associated voiceline
        if (!DialogueOption.VoicelineID.IsEmpty())
        {
            PlayVoiceline(DialogueOption.VoicelineID);
        }
        
        // Share information with nearby NPCs if it's important
        if (InteractionType == ENarr_NPCInteractionType::Warning || InteractionType == ENarr_NPCInteractionType::Emergency)
        {
            ShareInformationWithNearbyNPCs(DialogueOption.DialogueText, SocialShareRadius);
        }
        
        // Modify trust based on interaction type
        switch (InteractionType)
        {
            case ENarr_NPCInteractionType::Trade:
                ModifyTrustLevel(0.1f);
                break;
            case ENarr_NPCInteractionType::Information:
                ModifyTrustLevel(0.05f);
                break;
            case ENarr_NPCInteractionType::Casual:
                ModifyTrustLevel(0.02f);
                break;
            default:
                break;
        }
    }
}

void UNarr_NPCInteractionSystem::UpdateMoodBasedOnEvents(float DeltaTime)
{
    ENarr_NPCMoodState NewMood = CalculateNewMood(DeltaTime);
    
    if (NewMood != CurrentMood)
    {
        UE_LOG(LogTemp, Log, TEXT("%s mood changed from %d to %d"), *NPCPersonality.NPCName, (int32)CurrentMood, (int32)NewMood);
        SetMoodState(NewMood);
    }
}

void UNarr_NPCInteractionSystem::SetMoodState(ENarr_NPCMoodState NewMood)
{
    CurrentMood = NewMood;
    
    // Adjust behavior based on new mood
    switch (CurrentMood)
    {
        case ENarr_NPCMoodState::Fearful:
        case ENarr_NPCMoodState::Panicked:
            InteractionRange *= 0.7f; // Reduce interaction range when scared
            break;
        case ENarr_NPCMoodState::Friendly:
            InteractionRange *= 1.3f; // Increase interaction range when friendly
            break;
        case ENarr_NPCMoodState::Hostile:
            InteractionRange *= 0.5f; // Minimal interaction when hostile
            break;
        default:
            InteractionRange = 800.0f; // Reset to default
            break;
    }
}

void UNarr_NPCInteractionSystem::ModifyTrustLevel(float TrustChange)
{
    CurrentTrustLevel = FMath::Clamp(CurrentTrustLevel + TrustChange, 0.0f, 1.0f);
    NPCPersonality.TrustLevel = CurrentTrustLevel;
    
    UE_LOG(LogTemp, Log, TEXT("%s trust level: %.2f"), *NPCPersonality.NPCName, CurrentTrustLevel);
}

void UNarr_NPCInteractionSystem::RespondToDinosaurThreat(AActor* DinosaurActor, float ThreatLevel)
{
    if (!DinosaurActor)
    {
        return;
    }

    if (ThreatLevel > NPCPersonality.FearThreshold)
    {
        SetMoodState(ENarr_NPCMoodState::Panicked);
        
        // Share emergency information
        FString ThreatInfo = FString::Printf(TEXT("Dangerous dinosaur spotted at %s!"), 
            *DinosaurActor->GetActorLocation().ToString());
        ShareInformationWithNearbyNPCs(ThreatInfo, SocialShareRadius * 1.5f);
        
        // Play emergency alert
        PlayEmergencyAlert(TEXT("Massive predator detected! Seek immediate shelter!"));
    }
    else if (ThreatLevel > 0.3f)
    {
        SetMoodState(ENarr_NPCMoodState::Cautious);
    }
}

void UNarr_NPCInteractionSystem::RespondToWeatherChange(float Intensity)
{
    if (Intensity > 0.8f)
    {
        SetMoodState(ENarr_NPCMoodState::Fearful);
        ShareInformationWithNearbyNPCs(TEXT("Severe weather approaching! Seek shelter immediately!"), SocialShareRadius);
    }
    else if (Intensity > 0.5f)
    {
        SetMoodState(ENarr_NPCMoodState::Cautious);
    }
}

void UNarr_NPCInteractionSystem::RespondToPlayerActions(ATranspersonalCharacter* Player, const FString& ActionType)
{
    if (!Player)
    {
        return;
    }

    if (ActionType == TEXT("Aggressive"))
    {
        ModifyTrustLevel(-0.2f);
        if (CurrentTrustLevel < 0.3f)
        {
            SetMoodState(ENarr_NPCMoodState::Hostile);
        }
    }
    else if (ActionType == TEXT("Helpful"))
    {
        ModifyTrustLevel(0.15f);
        SetMoodState(ENarr_NPCMoodState::Friendly);
    }
    else if (ActionType == TEXT("Trade"))
    {
        ModifyTrustLevel(0.1f);
    }
}

void UNarr_NPCInteractionSystem::ShareInformationWithNearbyNPCs(const FString& Information, float ShareRadius)
{
    if (!GetOwner())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }
        
        UNarr_NPCInteractionSystem* OtherNPC = Actor->FindComponentByClass<UNarr_NPCInteractionSystem>();
        if (OtherNPC)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= ShareRadius)
            {
                OtherNPC->ReceiveInformationFromNPC(this, Information);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s shared information: %s"), *NPCPersonality.NPCName, *Information);
}

void UNarr_NPCInteractionSystem::ReceiveInformationFromNPC(UNarr_NPCInteractionSystem* SourceNPC, const FString& Information)
{
    if (!SourceNPC)
    {
        return;
    }

    SharedInformation.AddUnique(Information);
    
    // React to the information
    if (Information.Contains(TEXT("predator")) || Information.Contains(TEXT("dangerous")))
    {
        SetMoodState(ENarr_NPCMoodState::Fearful);
    }
    else if (Information.Contains(TEXT("weather")) || Information.Contains(TEXT("storm")))
    {
        SetMoodState(ENarr_NPCMoodState::Cautious);
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s received info from %s: %s"), 
        *NPCPersonality.NPCName, *SourceNPC->NPCPersonality.NPCName, *Information);
}

void UNarr_NPCInteractionSystem::FormGroupWithNearbyNPCs(float GroupRadius)
{
    UpdateNearbyNPCs();
    
    if (NearbyNPCs.Num() >= 2)
    {
        bInGroup = true;
        
        // Increase trust and reduce fear when in group
        ModifyTrustLevel(0.1f);
        if (CurrentMood == ENarr_NPCMoodState::Fearful)
        {
            SetMoodState(ENarr_NPCMoodState::Cautious);
        }
        
        UE_LOG(LogTemp, Log, TEXT("%s formed group with %d NPCs"), *NPCPersonality.NPCName, NearbyNPCs.Num());
    }
}

void UNarr_NPCInteractionSystem::PlayVoiceline(const FString& VoicelineID)
{
    UE_LOG(LogTemp, Warning, TEXT("Playing voiceline: %s"), *VoicelineID);
    // TODO: Integrate with audio system to play actual voiceline
}

void UNarr_NPCInteractionSystem::PlayEmergencyAlert(const FString& AlertMessage)
{
    UE_LOG(LogTemp, Error, TEXT("EMERGENCY ALERT: %s"), *AlertMessage);
    // TODO: Play emergency sound and display UI alert
}

bool UNarr_NPCInteractionSystem::IsPlayerInRange(ATranspersonalCharacter* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange;
}

bool UNarr_NPCInteractionSystem::HasLineOfSight(ATranspersonalCharacter* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }

    FHitResult HitResult;
    FVector Start = GetOwner()->GetActorLocation();
    FVector End = Player->GetActorLocation();
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility
    );
    
    return !bHit || HitResult.GetActor() == Player;
}

float UNarr_NPCInteractionSystem::CalculatePlayerThreatLevel(ATranspersonalCharacter* Player) const
{
    if (!Player)
    {
        return 0.0f;
    }

    float ThreatLevel = 0.0f;
    
    // Base threat on distance (closer = more threatening)
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    if (Distance < 300.0f)
    {
        ThreatLevel += 0.5f;
    }
    
    // TODO: Add weapon detection, player health status, etc.
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void UNarr_NPCInteractionSystem::UpdateNearbyNPCs()
{
    NearbyNPCs.Empty();
    
    if (!GetOwner())
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
        {
            continue;
        }
        
        UNarr_NPCInteractionSystem* OtherNPC = Actor->FindComponentByClass<UNarr_NPCInteractionSystem>();
        if (OtherNPC)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= SocialShareRadius)
            {
                NearbyNPCs.Add(Actor);
            }
        }
    }
}

ENarr_NPCMoodState UNarr_NPCInteractionSystem::CalculateNewMood(float DeltaTime)
{
    ENarr_NPCMoodState NewMood = CurrentMood;
    
    // Gradually return to neutral mood over time
    if (CurrentMood != ENarr_NPCMoodState::Neutral)
    {
        static float MoodDecayTimer = 0.0f;
        MoodDecayTimer += DeltaTime;
        
        if (MoodDecayTimer > 30.0f) // 30 seconds to start mood decay
        {
            switch (CurrentMood)
            {
                case ENarr_NPCMoodState::Panicked:
                    NewMood = ENarr_NPCMoodState::Fearful;
                    break;
                case ENarr_NPCMoodState::Fearful:
                    NewMood = ENarr_NPCMoodState::Cautious;
                    break;
                case ENarr_NPCMoodState::Cautious:
                case ENarr_NPCMoodState::Hostile:
                    NewMood = ENarr_NPCMoodState::Neutral;
                    break;
                case ENarr_NPCMoodState::Friendly:
                    if (CurrentTrustLevel < 0.7f)
                    {
                        NewMood = ENarr_NPCMoodState::Neutral;
                    }
                    break;
                default:
                    break;
            }
            MoodDecayTimer = 0.0f;
        }
    }
    
    return NewMood;
}