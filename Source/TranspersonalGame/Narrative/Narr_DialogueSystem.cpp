#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create dialogue mesh component
    DialogueMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DialogueMesh"));
    DialogueMesh->SetupAttachment(RootComponent);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(500.0f);

    // Set default values
    InteractionRange = 500.0f;
    NPCName = TEXT("Survivor");
    CurrentBiome = ENarr_BiomeType::Savana;
    CurrentDialogueIndex = 0;
    bIsInDialogue = false;
    CurrentPlayer = nullptr;

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_DialogueSystem::OnInteractionSphereEndOverlap);
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDialogueEntries();
    InitializeStoryPoints();
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update dialogue system logic here if needed
    if (bIsInDialogue && CurrentPlayer)
    {
        // Check if player is still in range
        float Distance = FVector::Dist(GetActorLocation(), CurrentPlayer->GetActorLocation());
        if (Distance > InteractionRange * 1.5f)
        {
            EndDialogue();
        }
    }
}

void ANarr_DialogueSystem::StartDialogue(AActor* Player)
{
    if (!Player || bIsInDialogue)
    {
        return;
    }

    CurrentPlayer = Player;
    bIsInDialogue = true;
    CurrentDialogueIndex = 0;

    // Log dialogue start
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Starting dialogue with %s"), *NPCName);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, Message);
    }

    // Get appropriate dialogue for current biome
    TArray<FNarr_DialogueEntry> BiomeDialogue = GetDialogueForBiome(CurrentBiome);
    if (BiomeDialogue.Num() > 0)
    {
        FNarr_DialogueEntry CurrentEntry = BiomeDialogue[0];
        if (GEngine)
        {
            FString DialogueMessage = FString::Printf(TEXT("%s: %s"), *CurrentEntry.SpeakerName, *CurrentEntry.DialogueText);
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DialogueMessage);
        }
    }
}

void ANarr_DialogueSystem::EndDialogue()
{
    if (!bIsInDialogue)
    {
        return;
    }

    bIsInDialogue = false;
    CurrentPlayer = nullptr;
    CurrentDialogueIndex = 0;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Dialogue ended"));
    }
}

FNarr_DialogueEntry ANarr_DialogueSystem::GetCurrentDialogue() const
{
    TArray<FNarr_DialogueEntry> BiomeDialogue = GetDialogueForBiome(CurrentBiome);
    
    if (BiomeDialogue.IsValidIndex(CurrentDialogueIndex))
    {
        return BiomeDialogue[CurrentDialogueIndex];
    }

    // Return default dialogue if none found
    FNarr_DialogueEntry DefaultEntry;
    DefaultEntry.SpeakerName = NPCName;
    DefaultEntry.DialogueText = TEXT("The wilderness is dangerous. Stay alert, survivor.");
    return DefaultEntry;
}

void ANarr_DialogueSystem::ProgressStory(const FString& StoryPointID)
{
    for (FNarr_StoryProgression& StoryPoint : StoryPoints)
    {
        if (StoryPoint.StoryPointID == StoryPointID && !StoryPoint.bIsCompleted)
        {
            StoryPoint.bIsCompleted = true;
            
            if (GEngine)
            {
                FString Message = FString::Printf(TEXT("Story Progress: %s completed!"), *StoryPoint.Description);
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Message);
            }
            break;
        }
    }
}

bool ANarr_DialogueSystem::IsStoryPointCompleted(const FString& StoryPointID) const
{
    for (const FNarr_StoryProgression& StoryPoint : StoryPoints)
    {
        if (StoryPoint.StoryPointID == StoryPointID)
        {
            return StoryPoint.bIsCompleted;
        }
    }
    return false;
}

void ANarr_DialogueSystem::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueEntries.Add(NewEntry);
}

TArray<FNarr_DialogueEntry> ANarr_DialogueSystem::GetDialogueForBiome(ENarr_BiomeType Biome) const
{
    TArray<FNarr_DialogueEntry> BiomeSpecificDialogue;
    
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.RequiredBiome == Biome)
        {
            BiomeSpecificDialogue.Add(Entry);
        }
    }
    
    return BiomeSpecificDialogue;
}

void ANarr_DialogueSystem::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<ACharacter>())
    {
        if (GEngine)
        {
            FString Message = FString::Printf(TEXT("Press E to talk to %s"), *NPCName);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, Message);
        }
    }
}

void ANarr_DialogueSystem::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == CurrentPlayer)
    {
        EndDialogue();
    }
}

void ANarr_DialogueSystem::InitializeDialogueEntries()
{
    DialogueEntries.Empty();

    // Savana dialogue
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Kora"), TEXT("Welcome to the Savana, survivor. The grasslands stretch endlessly, but beware - predators hunt here."), ENarr_BiomeType::Savana, 0.2f));
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Kora"), TEXT("The Triceratops herds migrate through these plains. Follow them to find water sources."), ENarr_BiomeType::Savana, 0.5f));

    // Pantano dialogue
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Marsh Guide"), TEXT("The swamplands are treacherous. Watch for quicksand and stay on solid ground."), ENarr_BiomeType::Pantano, -0.3f));
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Marsh Guide"), TEXT("Ancient creatures lurk beneath the murky waters. Never drink directly from the swamp."), ENarr_BiomeType::Pantano, -0.5f));

    // Floresta dialogue
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Forest Ranger"), TEXT("The forest provides shelter and resources, but visibility is limited. Listen for danger."), ENarr_BiomeType::Floresta, 0.1f));
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Forest Ranger"), TEXT("Brachiosaurus feed on the tallest trees. They're gentle unless threatened."), ENarr_BiomeType::Floresta, 0.4f));

    // Deserto dialogue
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Desert Nomad"), TEXT("Water is life in the desert. Conserve every drop and seek shade during the heat."), ENarr_BiomeType::Deserto, -0.2f));
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Desert Nomad"), TEXT("The sand hides many secrets. Look for rock formations that offer protection."), ENarr_BiomeType::Deserto, 0.0f));

    // Montanha dialogue
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Mountain Climber"), TEXT("The peaks are cold and unforgiving. Prepare for harsh weather and thin air."), ENarr_BiomeType::Montanha, -0.1f));
    DialogueEntries.Add(CreateDialogueEntry(TEXT("Mountain Climber"), TEXT("Caves in the mountains offer shelter, but some are home to dangerous predators."), ENarr_BiomeType::Montanha, -0.4f));
}

void ANarr_DialogueSystem::InitializeStoryPoints()
{
    StoryPoints.Empty();

    FNarr_StoryProgression TutorialStart;
    TutorialStart.StoryPointID = TEXT("Tutorial_Start");
    TutorialStart.Description = TEXT("Learn basic survival skills");
    TutorialStart.bIsCompleted = false;
    TutorialStart.CompletionReward = 10.0f;
    StoryPoints.Add(TutorialStart);

    FNarr_StoryProgression FirstHunt;
    FirstHunt.StoryPointID = TEXT("First_Hunt");
    FirstHunt.Description = TEXT("Successfully track and observe your first dinosaur");
    FirstHunt.bIsCompleted = false;
    FirstHunt.Prerequisites.Add(TEXT("Tutorial_Start"));
    FirstHunt.CompletionReward = 25.0f;
    StoryPoints.Add(FirstHunt);

    FNarr_StoryProgression TribeDiscovery;
    TribeDiscovery.StoryPointID = TEXT("Tribe_Discovery");
    TribeDiscovery.Description = TEXT("Find other survivors and establish contact");
    TribeDiscovery.bIsCompleted = false;
    TribeDiscovery.Prerequisites.Add(TEXT("First_Hunt"));
    TribeDiscovery.CompletionReward = 50.0f;
    StoryPoints.Add(TribeDiscovery);

    FNarr_StoryProgression AncientRuins;
    AncientRuins.StoryPointID = TEXT("Ancient_Ruins");
    AncientRuins.Description = TEXT("Discover prehistoric artifacts and ancient knowledge");
    AncientRuins.bIsCompleted = false;
    AncientRuins.Prerequisites.Add(TEXT("Tribe_Discovery"));
    AncientRuins.CompletionReward = 75.0f;
    StoryPoints.Add(AncientRuins);

    FNarr_StoryProgression FinalChallenge;
    FinalChallenge.StoryPointID = TEXT("Final_Challenge");
    FinalChallenge.Description = TEXT("Confront the apex predator and prove your mastery");
    FinalChallenge.bIsCompleted = false;
    FinalChallenge.Prerequisites.Add(TEXT("Ancient_Ruins"));
    FinalChallenge.CompletionReward = 100.0f;
    StoryPoints.Add(FinalChallenge);
}

FNarr_DialogueEntry ANarr_DialogueSystem::CreateDialogueEntry(const FString& Speaker, const FString& Text, ENarr_BiomeType Biome, float Tone)
{
    FNarr_DialogueEntry Entry;
    Entry.SpeakerName = Speaker;
    Entry.DialogueText = Text;
    Entry.RequiredBiome = Biome;
    Entry.EmotionalTone = Tone;
    
    // Add some context-appropriate responses
    if (Tone < -0.3f)
    {
        Entry.PlayerResponses.Add(TEXT("I understand the danger."));
        Entry.PlayerResponses.Add(TEXT("How can I stay safe?"));
    }
    else if (Tone > 0.3f)
    {
        Entry.PlayerResponses.Add(TEXT("Thank you for the guidance."));
        Entry.PlayerResponses.Add(TEXT("Tell me more."));
    }
    else
    {
        Entry.PlayerResponses.Add(TEXT("I see."));
        Entry.PlayerResponses.Add(TEXT("What else should I know?"));
    }
    
    return Entry;
}