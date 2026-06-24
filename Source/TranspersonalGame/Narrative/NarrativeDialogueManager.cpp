#include "NarrativeDialogueManager.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ANarrativeDialogueManager::ANarrativeDialogueManager()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsDialoguePlaying = false;
    DialogueTimeRemaining = 0.0f;
    ProximityCheckInterval = 0.5f;
    TimeSinceLastProximityCheck = 0.0f;
    CachedPlayerPawn = nullptr;
}

void ANarrativeDialogueManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultZones();
    CachedPlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void ANarrativeDialogueManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update dialogue timer
    UpdateDialogueTimer(DeltaTime);

    // Proximity check at interval
    TimeSinceLastProximityCheck += DeltaTime;
    if (TimeSinceLastProximityCheck >= ProximityCheckInterval)
    {
        TimeSinceLastProximityCheck = 0.0f;
        if (CachedPlayerPawn)
        {
            CheckProximityTriggers(CachedPlayerPawn->GetActorLocation());
        }
    }
}

void ANarrativeDialogueManager::TriggerDialogueLine(const FNarr_DialogueLine& Line)
{
    if (bIsDialoguePlaying && DialogueTimeRemaining > 1.0f)
    {
        // Don't interrupt current line unless it's nearly done
        return;
    }

    CurrentActiveLine = Line;
    bIsDialoguePlaying = true;
    DialogueTimeRemaining = Line.DisplayDuration;

    UE_LOG(LogTemp, Log, TEXT("[Narrative] %s: %s"), *Line.CharacterName, *Line.LineText);
}

void ANarrativeDialogueManager::CheckProximityTriggers(FVector PlayerLocation)
{
    for (FNarr_DialogueZone& Zone : DialogueZones)
    {
        if (Zone.bOneShot && Zone.bHasTriggered)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Zone.ZoneLocation);
        if (Distance <= Zone.TriggerRadius && Zone.DialogueLines.Num() > 0)
        {
            // Pick first unplayed line or random
            const FNarr_DialogueLine& Line = Zone.DialogueLines[0];
            TriggerDialogueLine(Line);

            if (Zone.bOneShot)
            {
                Zone.bHasTriggered = true;
            }
        }
    }
}

void ANarrativeDialogueManager::RegisterDialogueZone(const FNarr_DialogueZone& Zone)
{
    // Check for duplicate zone ID
    for (const FNarr_DialogueZone& Existing : DialogueZones)
    {
        if (Existing.ZoneID == Zone.ZoneID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Narrative] Zone %s already registered"), *Zone.ZoneID);
            return;
        }
    }
    DialogueZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("[Narrative] Registered zone: %s at %s"), *Zone.ZoneID, *Zone.ZoneLocation.ToString());
}

void ANarrativeDialogueManager::ClearActiveDialogue()
{
    bIsDialoguePlaying = false;
    DialogueTimeRemaining = 0.0f;
    CurrentActiveLine = FNarr_DialogueLine();
}

void ANarrativeDialogueManager::PopulateDefaultDialogueZones()
{
    DialogueZones.Empty();
    InitializeDefaultZones();
    UE_LOG(LogTemp, Log, TEXT("[Narrative] Populated %d default dialogue zones"), DialogueZones.Num());
}

void ANarrativeDialogueManager::InitializeDefaultZones()
{
    // === ZONE 1: T-Rex Valley — Tracker Elder warning ===
    {
        FNarr_DialogueZone Zone;
        Zone.ZoneID = TEXT("TRex_Valley_Warning");
        Zone.ZoneLocation = FVector(2000.0f, 2500.0f, 200.0f);
        Zone.TriggerRadius = 800.0f;
        Zone.bOneShot = true;

        FNarr_DialogueLine Line;
        Line.CharacterName = TEXT("Tracker Elder");
        Line.LineText = TEXT("The great beast moves through the valley below. See how it pauses, lifts its head, tests the wind. It knows we are here. Stay low. Stay still. We hunt only when the moment is right — not before.");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344157638_Tracker_Elder.mp3");
        Line.DisplayDuration = 13.0f;
        Line.TriggerType = ENarr_DialogueTriggerType::DinoEncounter;
        Zone.DialogueLines.Add(Line);

        DialogueZones.Add(Zone);
    }

    // === ZONE 2: Raptor River — Tribe Leader warning ===
    {
        FNarr_DialogueZone Zone;
        Zone.ZoneID = TEXT("Raptor_River_Warning");
        Zone.ZoneLocation = FVector(-1500.0f, 3000.0f, 200.0f);
        Zone.TriggerRadius = 600.0f;
        Zone.bOneShot = true;

        FNarr_DialogueLine Line;
        Line.CharacterName = TEXT("Tribe Leader Kael");
        Line.LineText = TEXT("We lost two hunters at the river crossing. The raptors hunt in packs — they are smart, they flank, they wait. If you go alone, you die alone. We move together or we do not move at all.");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344176100_Tribe_Leader_Kael.mp3");
        Line.DisplayDuration = 13.0f;
        Line.TriggerType = ENarr_DialogueTriggerType::DinoEncounter;
        Zone.DialogueLines.Add(Line);

        DialogueZones.Add(Zone);
    }

    // === ZONE 3: Camp Entrance — Elder narrator lore ===
    {
        FNarr_DialogueZone Zone;
        Zone.ZoneID = TEXT("Camp_Entrance_Lore");
        Zone.ZoneLocation = FVector(0.0f, 0.0f, 200.0f);
        Zone.TriggerRadius = 500.0f;
        Zone.bOneShot = true;

        FNarr_DialogueLine Line;
        Line.CharacterName = TEXT("Elder Narrator");
        Line.LineText = TEXT("This place was not always safe. Before the great hunt, before we learned to read the signs — the prints in the mud, the broken branches, the silence of birds — many of our people did not return. The land teaches those who listen. The rest it buries.");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344199704_Elder_Narrator.mp3");
        Line.DisplayDuration = 17.0f;
        Line.TriggerType = ENarr_DialogueTriggerType::ProximityEnter;
        Zone.DialogueLines.Add(Line);

        DialogueZones.Add(Zone);
    }

    // === ZONE 4: Escape Route — Scout Mira urgent ===
    {
        FNarr_DialogueZone Zone;
        Zone.ZoneID = TEXT("TRex_Escape_Route");
        Zone.ZoneLocation = FVector(1500.0f, 1000.0f, 200.0f);
        Zone.TriggerRadius = 400.0f;
        Zone.bOneShot = false; // Can retrigger — it's an escape route

        FNarr_DialogueLine Line;
        Line.CharacterName = TEXT("Scout Mira");
        Line.LineText = TEXT("Move. Now. Do not look back. The T-Rex tracks scent, not sight — if we reach the river, the water breaks the trail. Run!");
        Line.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1782344208387_Scout_Mira.mp3");
        Line.DisplayDuration = 8.0f;
        Line.TriggerType = ENarr_DialogueTriggerType::DinoEncounter;
        Zone.DialogueLines.Add(Line);

        DialogueZones.Add(Zone);
    }

    UE_LOG(LogTemp, Log, TEXT("[Narrative] Initialized %d dialogue zones"), DialogueZones.Num());
}

void ANarrativeDialogueManager::UpdateDialogueTimer(float DeltaTime)
{
    if (!bIsDialoguePlaying) return;

    DialogueTimeRemaining -= DeltaTime;
    if (DialogueTimeRemaining <= 0.0f)
    {
        ClearActiveDialogue();
    }
}
