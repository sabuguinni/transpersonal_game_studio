#include "NarrativeQuestBridge.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarrativeQuestBridge::UNarrativeQuestBridge()
{
    PrimaryComponentTick.bCanEverTick = true;
    NarrativeCheckInterval = 2.0f;
    bNarrativeSystemActive = true;
    HuntSystemRef = nullptr;
    PlayerCharacterRef = nullptr;
}

void UNarrativeQuestBridge::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character reference
    if (AActor* Owner = GetOwner())
    {
        PlayerCharacterRef = Cast<ATranspersonalCharacter>(Owner);
        if (!PlayerCharacterRef)
        {
            PlayerCharacterRef = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        }
    }
    
    // Initialize quest narratives
    InitializeQuestNarratives();
    
    // Start narrative checking timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            NarrativeTimerHandle,
            this,
            &UNarrativeQuestBridge::CheckQuestProgress,
            NarrativeCheckInterval,
            true
        );
    }
}

void UNarrativeQuestBridge::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Additional real-time narrative checks can go here
}

void UNarrativeQuestBridge::InitializeQuestNarratives()
{
    QuestNarratives.Empty();
    
    // Hunt Introduction Narratives
    FNarr_QuestNarrative VelociraptorIntro;
    VelociraptorIntro.NarrativeType = ENarr_QuestNarrativeType::HuntIntroduction;
    VelociraptorIntro.DialogueText = TEXT("Pack hunters spotted in the savanna. Velociraptors are clever and fast. Strike quickly before they surround you.");
    VelociraptorIntro.SpeakerName = TEXT("Tribal Scout");
    VelociraptorIntro.DialogueDuration = 5.0f;
    VelociraptorIntro.bIsUrgent = false;
    QuestNarratives.Add(VelociraptorIntro);
    
    FNarr_QuestNarrative TrexIntro;
    TrexIntro.NarrativeType = ENarr_QuestNarrativeType::HuntIntroduction;
    TrexIntro.DialogueText = TEXT("The earth trembles. The apex predator walks among us. Only the bravest hunters dare face the Tyrant King.");
    TrexIntro.SpeakerName = TEXT("Elder Hunter");
    TrexIntro.DialogueDuration = 6.0f;
    TrexIntro.bIsUrgent = true;
    QuestNarratives.Add(TrexIntro);
    
    // Hunt Completion Narratives
    FNarr_QuestNarrative HuntSuccess;
    HuntSuccess.NarrativeType = ENarr_QuestNarrativeType::HuntCompletion;
    HuntSuccess.DialogueText = TEXT("The hunt is complete. Your skill grows with each victory. The tribe will feast tonight.");
    HuntSuccess.SpeakerName = TEXT("Elder Hunter");
    HuntSuccess.DialogueDuration = 4.0f;
    HuntSuccess.bIsUrgent = false;
    QuestNarratives.Add(HuntSuccess);
    
    // Territory Warning Narratives
    FNarr_QuestNarrative TerritoryWarning;
    TerritoryWarning.NarrativeType = ENarr_QuestNarrativeType::TerritoryWarning;
    TerritoryWarning.DialogueText = TEXT("You enter dangerous territory. Ancient predators claim this land. Tread carefully, young hunter.");
    TerritoryWarning.SpeakerName = TEXT("Tribal Scout");
    TerritoryWarning.DialogueDuration = 4.5f;
    TerritoryWarning.bIsUrgent = true;
    QuestNarratives.Add(TerritoryWarning);
}

void UNarrativeQuestBridge::TriggerHuntNarrative(ENarr_QuestNarrativeType NarrativeType, const FString& HuntTarget)
{
    if (!bNarrativeSystemActive) return;
    
    for (const FNarr_QuestNarrative& Narrative : QuestNarratives)
    {
        if (Narrative.NarrativeType == NarrativeType)
        {
            // Customize dialogue based on hunt target
            FNarr_QuestNarrative CustomNarrative = Narrative;
            
            if (HuntTarget.Contains(TEXT("Velociraptor")))
            {
                CustomNarrative.DialogueText = TEXT("Pack hunters spotted in the savanna. Velociraptors are clever and fast. Strike quickly before they surround you.");
            }
            else if (HuntTarget.Contains(TEXT("TRex")) || HuntTarget.Contains(TEXT("Tyrannosaurus")))
            {
                CustomNarrative.DialogueText = TEXT("The earth trembles. The apex predator walks among us. Only the bravest hunters dare face the Tyrant King.");
                CustomNarrative.bIsUrgent = true;
            }
            else if (HuntTarget.Contains(TEXT("Triceratops")))
            {
                CustomNarrative.DialogueText = TEXT("Armored giants graze in the distance. Their horns can pierce any hunter. Approach from behind, avoid the charge.");
            }
            
            PlayNarrative(CustomNarrative);
            break;
        }
    }
}

void UNarrativeQuestBridge::CheckQuestProgress()
{
    if (!HuntSystemRef || !PlayerCharacterRef) return;
    
    // Check if player is near dangerous areas and trigger warnings
    FVector PlayerLocation = PlayerCharacterRef->GetActorLocation();
    
    // T-Rex territory warning (around 0,0,0 - savanna center)
    float DistanceToTrexTerritory = FVector::Dist(PlayerLocation, FVector(0, 0, 0));
    if (DistanceToTrexTerritory < 5000.0f) // Within 50 meters of T-Rex territory
    {
        TriggerHuntNarrative(ENarr_QuestNarrativeType::TerritoryWarning, TEXT("TRex"));
    }
    
    // Check hunt progress and trigger appropriate narratives
    // This would integrate with the hunt system to provide contextual dialogue
}

void UNarrativeQuestBridge::PlayNarrative(const FNarr_QuestNarrative& Narrative)
{
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), *Narrative.SpeakerName, *Narrative.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, Narrative.DialogueDuration, 
            Narrative.bIsUrgent ? FColor::Red : FColor::Yellow, DisplayText);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative: [%s] %s"), *Narrative.SpeakerName, *Narrative.DialogueText);
}

FNarr_QuestNarrative UNarrativeQuestBridge::GetHuntIntroNarrative(const FString& DinosaurSpecies)
{
    FNarr_QuestNarrative DefaultNarrative;
    DefaultNarrative.NarrativeType = ENarr_QuestNarrativeType::HuntIntroduction;
    DefaultNarrative.SpeakerName = TEXT("Tribal Scout");
    DefaultNarrative.DialogueDuration = 4.0f;
    
    if (DinosaurSpecies.Contains(TEXT("Velociraptor")))
    {
        DefaultNarrative.DialogueText = TEXT("Pack hunters spotted. Velociraptors hunt in coordinated groups. Stay alert.");
    }
    else if (DinosaurSpecies.Contains(TEXT("TRex")))
    {
        DefaultNarrative.DialogueText = TEXT("The apex predator approaches. Face the Tyrant King with courage, young hunter.");
        DefaultNarrative.bIsUrgent = true;
    }
    else
    {
        DefaultNarrative.DialogueText = TEXT("A worthy hunt awaits. Prove your skill against the ancient beasts.");
    }
    
    return DefaultNarrative;
}

FNarr_QuestNarrative UNarrativeQuestBridge::GetHuntCompletionNarrative(const FString& DinosaurSpecies, int32 KillCount)
{
    FNarr_QuestNarrative CompletionNarrative;
    CompletionNarrative.NarrativeType = ENarr_QuestNarrativeType::HuntCompletion;
    CompletionNarrative.SpeakerName = TEXT("Elder Hunter");
    CompletionNarrative.DialogueDuration = 5.0f;
    CompletionNarrative.bIsUrgent = false;
    
    if (KillCount >= 3)
    {
        CompletionNarrative.DialogueText = TEXT("Exceptional hunting! Your prowess grows with each victory. The tribe honors your skill.");
    }
    else if (KillCount >= 1)
    {
        CompletionNarrative.DialogueText = TEXT("The hunt is complete. Your courage serves the tribe well. Rest and prepare for greater challenges.");
    }
    else
    {
        CompletionNarrative.DialogueText = TEXT("The hunt continues. Patience and persistence will lead to victory, young hunter.");
    }
    
    return CompletionNarrative;
}

void UNarrativeQuestBridge::SetHuntSystemReference(UQuest_DinosaurHuntSystem* HuntSystem)
{
    HuntSystemRef = HuntSystem;
    UE_LOG(LogTemp, Warning, TEXT("NarrativeQuestBridge: Hunt system reference set"));
}