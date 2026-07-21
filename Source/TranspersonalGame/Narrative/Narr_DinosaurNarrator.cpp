#include "Narr_DinosaurNarrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

ANarr_DinosaurNarrator::ANarr_DinosaurNarrator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    NarrationAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NarrationAudioComponent"));
    RootComponent = NarrationAudioComponent;
    NarrationAudioComponent->bAutoActivate = false;
    NarrationAudioComponent->SetVolumeMultiplier(0.8f);

    // Initialize default values
    bNarrationEnabled = true;
    PlayerDetectionRadius = 5000.0f;
    NarrationCooldown = 10.0f;
    LastNarrationTime = 0.0f;
}

void ANarr_DinosaurNarrator::BeginPlay()
{
    Super::BeginPlay();
    
    LoadDefaultDinosaurNarrations();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Narrator initialized with %d narrations"), DinosaurNarrations.Num());
}

void ANarr_DinosaurNarrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bNarrationEnabled)
    {
        return;
    }

    // Check for nearby dinosaurs and trigger appropriate narration
    TArray<AActor*> NearbyDinosaurs = FindNearbyDinosaurs();
    
    for (AActor* DinosaurActor : NearbyDinosaurs)
    {
        if (!DinosaurActor)
        {
            continue;
        }

        FString ActorName = DinosaurActor->GetName().ToLower();
        EDinosaurSpecies DinosaurType = EDinosaurSpecies::TRex;

        // Determine dinosaur type from actor name
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("tyrannosaurus")))
        {
            DinosaurType = EDinosaurSpecies::TRex;
        }
        else if (ActorName.Contains(TEXT("veloci")) || ActorName.Contains(TEXT("raptor")))
        {
            DinosaurType = EDinosaurSpecies::Velociraptor;
        }
        else if (ActorName.Contains(TEXT("tricera")))
        {
            DinosaurType = EDinosaurSpecies::Triceratops;
        }
        else if (ActorName.Contains(TEXT("brachi")))
        {
            DinosaurType = EDinosaurSpecies::Brachiosaurus;
        }
        else if (ActorName.Contains(TEXT("ankylo")))
        {
            DinosaurType = EDinosaurSpecies::Ankylosaurus;
        }
        else if (ActorName.Contains(TEXT("parasauro")))
        {
            DinosaurType = EDinosaurSpecies::Parasaurolophus;
        }

        // Calculate distance to player
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
                TriggerNarrationForDinosaur(DinosaurType, Distance);
            }
        }
    }
}

void ANarr_DinosaurNarrator::TriggerNarrationForDinosaur(EDinosaurSpecies DinosaurType, float Distance)
{
    if (!CanPlayNarration(DinosaurType))
    {
        return;
    }

    // Find appropriate narration for this dinosaur type
    for (const FNarr_DinosaurNarration& Narration : DinosaurNarrations)
    {
        if (Narration.DinosaurType == DinosaurType && Distance <= Narration.TriggerDistance)
        {
            PlayNarration(Narration);
            
            if (Narration.bPlayOnce)
            {
                PlayedNarrations.Add(DinosaurType, true);
            }
            
            LastNarrationTime = GetWorld()->GetTimeSeconds();
            OnDinosaurNarrationTriggered.Broadcast(DinosaurType, Narration.NarrationText);
            
            UE_LOG(LogTemp, Log, TEXT("Triggered narration for dinosaur type %d: %s"), 
                   (int32)DinosaurType, *Narration.NarrationText);
            break;
        }
    }
}

void ANarr_DinosaurNarrator::AddDinosaurNarration(const FNarr_DinosaurNarration& NewNarration)
{
    DinosaurNarrations.Add(NewNarration);
    UE_LOG(LogTemp, Log, TEXT("Added narration for dinosaur type %d"), (int32)NewNarration.DinosaurType);
}

void ANarr_DinosaurNarrator::LoadDefaultDinosaurNarrations()
{
    // T-Rex narrations
    FNarr_DinosaurNarration TRexClose;
    TRexClose.DinosaurType = EDinosaurSpecies::TRex;
    TRexClose.NarrationText = TEXT("The apex predator approaches. Tyrannosaurus Rex - thirteen meters of pure killing power. Its bite force exceeds twelve thousand pounds per square inch.");
    TRexClose.TriggerDistance = 2000.0f;
    TRexClose.EmotionalTone = ENarr_EmotionalTone::Fearful;
    TRexClose.bPlayOnce = true;
    DinosaurNarrations.Add(TRexClose);

    FNarr_DinosaurNarration TRexFar;
    TRexFar.DinosaurType = EDinosaurSpecies::TRex;
    TRexFar.NarrationText = TEXT("Thunderous footsteps echo across the landscape. A T-Rex stalks its territory. Stay downwind and avoid sudden movements.");
    TRexFar.TriggerDistance = 5000.0f;
    TRexFar.EmotionalTone = ENarr_EmotionalTone::Tense;
    TRexFar.bPlayOnce = false;
    DinosaurNarrations.Add(TRexFar);

    // Velociraptor narrations
    FNarr_DinosaurNarration RaptorPack;
    RaptorPack.DinosaurType = EDinosaurSpecies::Velociraptor;
    RaptorPack.NarrationText = TEXT("Velociraptors - the perfect pack hunters. They communicate through sharp calls and coordinate their attacks with lethal precision.");
    RaptorPack.TriggerDistance = 1500.0f;
    RaptorPack.EmotionalTone = ENarr_EmotionalTone::Urgent;
    RaptorPack.bPlayOnce = true;
    DinosaurNarrations.Add(RaptorPack);

    // Triceratops narrations
    FNarr_DinosaurNarration TriceratopsHerd;
    TriceratopsHerd.DinosaurType = EDinosaurSpecies::Triceratops;
    TriceratopsHerd.NarrationText = TEXT("Triceratops - heavily armored herbivores. Their three-horned skulls can gore predators with devastating force. Approach the herd carefully.");
    TriceratopsHerd.TriggerDistance = 2500.0f;
    TriceratopsHerd.EmotionalTone = ENarr_EmotionalTone::Cautious;
    TriceratopsHerd.bPlayOnce = true;
    DinosaurNarrations.Add(TriceratopsHerd);

    // Brachiosaurus narrations
    FNarr_DinosaurNarration BrachiosaurusGentle;
    BrachiosaurusGentle.DinosaurType = EDinosaurSpecies::Brachiosaurus;
    BrachiosaurusGentle.NarrationText = TEXT("The gentle giants of the Cretaceous. Brachiosaurus towers above the forest canopy, feeding on the highest leaves. They pose no threat unless provoked.");
    BrachiosaurusGentle.TriggerDistance = 3000.0f;
    BrachiosaurusGentle.EmotionalTone = ENarr_EmotionalTone::Peaceful;
    BrachiosaurusGentle.bPlayOnce = true;
    DinosaurNarrations.Add(BrachiosaurusGentle);

    // Ankylosaurus narrations
    FNarr_DinosaurNarration AnkylosaurusArmored;
    AnkylosaurusArmored.DinosaurType = EDinosaurSpecies::Ankylosaurus;
    AnkylosaurusArmored.NarrationText = TEXT("Ankylosaurus - a living fortress. Its armored hide deflects most attacks, and that club tail can shatter bones. Keep your distance from the rear.");
    AnkylosaurusArmored.TriggerDistance = 2000.0f;
    AnkylosaurusArmored.EmotionalTone = ENarr_EmotionalTone::Respectful;
    AnkylosaurusArmored.bPlayOnce = true;
    DinosaurNarrations.Add(AnkylosaurusArmored);

    // Parasaurolophus narrations
    FNarr_DinosaurNarration ParasaurolophusHerd;
    ParasaurolophusHerd.DinosaurType = EDinosaurSpecies::Parasaurolophus;
    ParasaurolophusHerd.NarrationText = TEXT("Parasaurolophus herds communicate through haunting trumpet calls. These duck-billed herbivores are alert and will flee at the first sign of danger.");
    ParasaurolophusHerd.TriggerDistance = 2500.0f;
    ParasaurolophusHerd.EmotionalTone = ENarr_EmotionalTone::Informative;
    ParasaurolophusHerd.bPlayOnce = true;
    DinosaurNarrations.Add(ParasaurolophusHerd);

    UE_LOG(LogTemp, Log, TEXT("Loaded %d default dinosaur narrations"), DinosaurNarrations.Num());
}

bool ANarr_DinosaurNarrator::CanPlayNarration(EDinosaurSpecies DinosaurType) const
{
    // Check cooldown
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastNarrationTime < NarrationCooldown)
    {
        return false;
    }

    // Check if this narration was already played (for one-time narrations)
    if (const bool* bPlayed = PlayedNarrations.Find(DinosaurType))
    {
        if (*bPlayed)
        {
            return false;
        }
    }

    return true;
}

void ANarr_DinosaurNarrator::PlayNarration(const FNarr_DinosaurNarration& Narration)
{
    if (!NarrationAudioComponent)
    {
        return;
    }

    // Play voice clip if available
    if (Narration.VoiceClip)
    {
        NarrationAudioComponent->SetSound(Narration.VoiceClip);
        NarrationAudioComponent->Play();
    }

    UE_LOG(LogTemp, Log, TEXT("Playing dinosaur narration: %s"), *Narration.NarrationText);
}

TArray<AActor*> ANarr_DinosaurNarrator::FindNearbyDinosaurs() const
{
    TArray<AActor*> NearbyDinosaurs;

    if (!GetWorld())
    {
        return NearbyDinosaurs;
    }

    // Get player location
    FVector PlayerLocation = GetActorLocation();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }

    // Find all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }

        FString ActorName = Actor->GetName().ToLower();
        
        // Check if this is a dinosaur actor
        bool bIsDinosaur = ActorName.Contains(TEXT("trex")) ||
                          ActorName.Contains(TEXT("veloci")) ||
                          ActorName.Contains(TEXT("raptor")) ||
                          ActorName.Contains(TEXT("tricera")) ||
                          ActorName.Contains(TEXT("brachi")) ||
                          ActorName.Contains(TEXT("ankylo")) ||
                          ActorName.Contains(TEXT("parasauro")) ||
                          ActorName.Contains(TEXT("tyrannosaurus"));

        if (bIsDinosaur)
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= PlayerDetectionRadius)
            {
                NearbyDinosaurs.Add(Actor);
            }
        }
    }

    return NearbyDinosaurs;
}