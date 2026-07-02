// DialogueSystem.cpp — Narrative & Dialogue Agent #15
#include "DialogueSystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_NPCDialogueActor
// ─────────────────────────────────────────────────────────────────────────────

ANarr_NPCDialogueActor::ANarr_NPCDialogueActor()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = TriggerSphere;

    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_NPCDialogueActor::OnPlayerEnterRange);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_NPCDialogueActor::OnPlayerExitRange);
}

void ANarr_NPCDialogueActor::BeginPlay()
{
    Super::BeginPlay();
    TriggerSphere->SetSphereRadius(TriggerRadius);
    ActiveLineIndex = 0;
    LineDisplayTimer = 0.0f;
    bPlayerInRange = false;
}

void ANarr_NPCDialogueActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bPlayerInRange && LineDisplayTimer > 0.0f)
    {
        LineDisplayTimer -= DeltaTime;
        if (LineDisplayTimer <= 0.0f)
        {
            // Auto-advance to next line after display duration
            if (ActiveLineIndex < DialogueSet.Lines.Num() - 1)
            {
                ActiveLineIndex++;
                FNarr_DialogueLine& Line = DialogueSet.Lines[ActiveLineIndex];
                Line.bHasBeenPlayed = true;
                LineDisplayTimer = Line.DisplayDuration;
            }
        }
    }
}

void ANarr_NPCDialogueActor::TriggerNextLine()
{
    if (DialogueSet.Lines.Num() == 0) return;

    if (ActiveLineIndex < DialogueSet.Lines.Num())
    {
        FNarr_DialogueLine& Line = DialogueSet.Lines[ActiveLineIndex];
        Line.bHasBeenPlayed = true;
        LineDisplayTimer = Line.DisplayDuration;

        if (ActiveLineIndex < DialogueSet.Lines.Num() - 1)
        {
            ActiveLineIndex++;
        }
    }
}

FNarr_DialogueLine ANarr_NPCDialogueActor::GetCurrentLine() const
{
    if (DialogueSet.Lines.IsValidIndex(ActiveLineIndex))
    {
        return DialogueSet.Lines[ActiveLineIndex];
    }
    return FNarr_DialogueLine();
}

bool ANarr_NPCDialogueActor::HasCraftingLine(const FString& ItemTag) const
{
    for (const FNarr_DialogueLine& Line : CraftingLinkedLines)
    {
        if (Line.RequiredItemTag.Equals(ItemTag, ESearchCase::IgnoreCase))
        {
            return true;
        }
    }
    return false;
}

FNarr_DialogueLine ANarr_NPCDialogueActor::GetCraftingLine(const FString& ItemTag) const
{
    for (const FNarr_DialogueLine& Line : CraftingLinkedLines)
    {
        if (Line.RequiredItemTag.Equals(ItemTag, ESearchCase::IgnoreCase))
        {
            return Line;
        }
    }
    return FNarr_DialogueLine();
}

void ANarr_NPCDialogueActor::ResetDialogue()
{
    ActiveLineIndex = 0;
    LineDisplayTimer = 0.0f;
    for (FNarr_DialogueLine& Line : DialogueSet.Lines)
    {
        Line.bHasBeenPlayed = false;
    }
}

int32 ANarr_NPCDialogueActor::GetTotalLines() const
{
    return DialogueSet.Lines.Num();
}

void ANarr_NPCDialogueActor::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = true;
        // Start showing first line
        if (DialogueSet.Lines.IsValidIndex(ActiveLineIndex))
        {
            LineDisplayTimer = DialogueSet.Lines[ActiveLineIndex].DisplayDuration;
            DialogueSet.Lines[ActiveLineIndex].bHasBeenPlayed = true;
        }
    }
}

void ANarr_NPCDialogueActor::OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
    {
        bPlayerInRange = false;
        LineDisplayTimer = 0.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ANarr_DialogueManager
// ─────────────────────────────────────────────────────────────────────────────

ANarr_DialogueManager::ANarr_DialogueManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANarr_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultJournalEntries();

    // Auto-discover NPC dialogue actors in the world
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANarr_NPCDialogueActor::StaticClass(), Found);
    for (AActor* A : Found)
    {
        ANarr_NPCDialogueActor* NPC = Cast<ANarr_NPCDialogueActor>(A);
        if (NPC)
        {
            RegisteredNPCs.Add(NPC);
        }
    }
}

bool ANarr_DialogueManager::UnlockJournalEntry(const FString& TriggerTag)
{
    for (FNarr_JournalEntry& Entry : JournalEntries)
    {
        if (!Entry.bIsUnlocked && Entry.UnlockTrigger.Equals(TriggerTag, ESearchCase::IgnoreCase))
        {
            Entry.bIsUnlocked = true;
            return true;
        }
    }
    return false;
}

TArray<FNarr_JournalEntry> ANarr_DialogueManager::GetUnlockedEntries() const
{
    TArray<FNarr_JournalEntry> Result;
    for (const FNarr_JournalEntry& Entry : JournalEntries)
    {
        if (Entry.bIsUnlocked)
        {
            Result.Add(Entry);
        }
    }
    return Result;
}

TArray<FNarr_JournalEntry> ANarr_DialogueManager::GetEntriesByCategory(ENarr_JournalCategory Category) const
{
    TArray<FNarr_JournalEntry> Result;
    for (const FNarr_JournalEntry& Entry : JournalEntries)
    {
        if (Entry.bIsUnlocked && Entry.Category == Category)
        {
            Result.Add(Entry);
        }
    }
    return Result;
}

void ANarr_DialogueManager::RegisterNPC(ANarr_NPCDialogueActor* NPC)
{
    if (NPC && !RegisteredNPCs.Contains(NPC))
    {
        RegisteredNPCs.Add(NPC);
    }
}

void ANarr_DialogueManager::OnPlayerCraftedItem(const FString& ItemTag)
{
    // Unlock crafting journal entry
    UnlockJournalEntry(FString::Printf(TEXT("Crafted%s"), *ItemTag));

    // Notify all NPCs that have a crafting-linked line for this item
    for (ANarr_NPCDialogueActor* NPC : RegisteredNPCs)
    {
        if (NPC && NPC->HasCraftingLine(ItemTag))
        {
            // NPC will serve crafting-specific dialogue next time player is in range
            UE_LOG(LogTemp, Log, TEXT("Narr: NPC %s has crafting line for %s"), *NPC->GetName(), *ItemTag);
        }
    }
}

void ANarr_DialogueManager::OnPlayerEncounteredDino(const FString& DinoSpecies)
{
    UnlockJournalEntry(FString::Printf(TEXT("Saw%s"), *DinoSpecies));
    UE_LOG(LogTemp, Log, TEXT("Narr: Journal entry unlocked for dino encounter: %s"), *DinoSpecies);
}

int32 ANarr_DialogueManager::GetUnlockedEntryCount() const
{
    int32 Count = 0;
    for (const FNarr_JournalEntry& Entry : JournalEntries)
    {
        if (Entry.bIsUnlocked) Count++;
    }
    return Count;
}

void ANarr_DialogueManager::RegisterDefaultJournalEntries()
{
    JournalEntries.Empty();

    // ── Survival entries ──────────────────────────────────────────────────────
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("First Night");
        E.EntryBody = TEXT("The cold comes fast after dark. I found shelter under a rock overhang. The sounds in the dark are constant — growls, distant crashes, things moving through the undergrowth. I must build a fire before the next nightfall.");
        E.Category = ENarr_JournalCategory::Survival;
        E.UnlockTrigger = TEXT("SurvivedFirstNight");
        JournalEntries.Add(E);
    }
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("Reading the Land");
        E.EntryBody = TEXT("The elder showed me: broken branches at shoulder height mean something large passed through recently. Claw marks on bark mean a predator marked territory. Flattened grass in a circle means a nest. Avoid all three.");
        E.Category = ENarr_JournalCategory::Survival;
        E.UnlockTrigger = TEXT("MetHunterElder");
        JournalEntries.Add(E);
    }

    // ── Crafting entries ──────────────────────────────────────────────────────
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("The Stone Axe");
        E.EntryBody = TEXT("Two sharp rocks, struck together at the right angle, produce a blade that holds an edge. Bind it to a straight stick with hide strips. The elder calls it the first tool — the one that separates the hunted from the hunter.");
        E.Category = ENarr_JournalCategory::Crafting;
        E.UnlockTrigger = TEXT("CraftedStoneAxe");
        JournalEntries.Add(E);
    }
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("Fire");
        E.EntryBody = TEXT("Three dry sticks, friction, patience. The fire keeps the night hunters away. They fear it — even the large ones. A campfire is not just warmth; it is a boundary that says: this space is claimed.");
        E.Category = ENarr_JournalCategory::Crafting;
        E.UnlockTrigger = TEXT("CraftedCampfire");
        JournalEntries.Add(E);
    }
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("Water Container");
        E.EntryBody = TEXT("A hollowed rock sealed with leaf resin holds water for half a day. Not elegant, but it means I can move away from the river without dying of thirst. The craftsman says the next step is clay — if I can find a riverbed deposit.");
        E.Category = ENarr_JournalCategory::Crafting;
        E.UnlockTrigger = TEXT("CraftedWaterContainer");
        JournalEntries.Add(E);
    }

    // ── Dinosaur lore entries ─────────────────────────────────────────────────
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("The Great Rex");
        E.EntryBody = TEXT("I saw it from the ridge. Twelve metres of muscle and bone, moving through the valley like a storm. It did not see me. The elder says they hunt by motion and sound, not smell. Stay still. Stay downwind. Do not run.");
        E.Category = ENarr_JournalCategory::Dinosaur;
        E.UnlockTrigger = TEXT("SawTRex");
        JournalEntries.Add(E);
    }
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("Raptor Pack");
        E.EntryBody = TEXT("Three of them, working together. One drew my attention from the front while two flanked from the sides. I only survived because I reached the cliff face — they would not climb. They are not just fast. They are intelligent.");
        E.Category = ENarr_JournalCategory::Dinosaur;
        E.UnlockTrigger = TEXT("SawRaptors");
        JournalEntries.Add(E);
    }
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("The Long-Necks");
        E.EntryBody = TEXT("The herd of long-necks is moving north. Hundreds of them, shaking the ground with every step. They are not dangerous unless you are beneath their feet. Follow at distance — where they go, there is always water.");
        E.Category = ENarr_JournalCategory::Dinosaur;
        E.UnlockTrigger = TEXT("SawBrachiosaurus");
        JournalEntries.Add(E);
    }

    // ── Territory entries ─────────────────────────────────────────────────────
    {
        FNarr_JournalEntry E;
        E.EntryTitle = TEXT("The Valley");
        E.EntryBody = TEXT("The valley runs north to south, cut by a river that never dries. The elder says our tribe has camped here for three generations. The eastern ridge is rex territory — we do not go there after dark. The western forest belongs to the pack hunters.");
        E.Category = ENarr_JournalCategory::Territory;
        E.UnlockTrigger = TEXT("ExploredValley");
        JournalEntries.Add(E);
    }

    UE_LOG(LogTemp, Log, TEXT("Narr: Registered %d default journal entries"), JournalEntries.Num());
}
