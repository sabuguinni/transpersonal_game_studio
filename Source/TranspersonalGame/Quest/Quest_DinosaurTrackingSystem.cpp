#include "Quest_DinosaurTrackingSystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AQuest_DinosaurTrackingSystem::AQuest_DinosaurTrackingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create tracking zone component
    TrackingZone = CreateDefaultSubobject<USphereComponent>(TEXT("TrackingZone"));
    RootComponent = TrackingZone;
    TrackingZone->SetSphereRadius(3000.0f);
    TrackingZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TrackingZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    TrackingZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create tracking marker
    TrackingMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrackingMarker"));
    TrackingMarker->SetupAttachment(RootComponent);
    
    // Set default mesh for tracking marker
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        TrackingMarker->SetStaticMesh(SphereMeshAsset.Object);
        TrackingMarker->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Initialize quest parameters
    CurrentObjective = EQuest_TrackingObjective::FindTracks;
    TargetSpecies = TEXT("TRex");
    RequiredObservations = 3;
    CompletedObservations = 0;
    TrackingRange = 5000.0f;
    bQuestActive = false;
    CurrentBiome = TEXT("Savana");

    // Bind overlap events
    TrackingZone->OnComponentBeginOverlap.AddDynamic(this, &AQuest_DinosaurTrackingSystem::OnTrackingZoneOverlapBegin);
    TrackingZone->OnComponentEndOverlap.AddDynamic(this, &AQuest_DinosaurTrackingSystem::OnTrackingZoneOverlapEnd);
}

void AQuest_DinosaurTrackingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeTrackingPoints();
    
    if (bQuestActive)
    {
        StartTrackingQuest(TargetSpecies, CurrentObjective, RequiredObservations);
    }
}

void AQuest_DinosaurTrackingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bQuestActive)
    {
        ScanForDinosaurs();
        ProcessTrackingObjective();
        UpdateTrackingMarkers();
    }
}

void AQuest_DinosaurTrackingSystem::StartTrackingQuest(const FString& Species, EQuest_TrackingObjective Objective, int32 RequiredCount)
{
    TargetSpecies = Species;
    CurrentObjective = Objective;
    RequiredObservations = RequiredCount;
    CompletedObservations = 0;
    bQuestActive = true;
    ActiveTracks.Empty();

    // Detect current biome based on actor location
    CurrentBiome = DetectCurrentBiome(GetActorLocation());

    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurTrackingSystem: Started tracking quest for %s in %s biome"), *Species, *CurrentBiome);

    if (GEngine)
    {
        FString QuestMessage = FString::Printf(TEXT("Tracking Quest Started: Find %s in %s biome (%d observations required)"), 
            *Species, *CurrentBiome, RequiredCount);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, QuestMessage);
    }
}

void AQuest_DinosaurTrackingSystem::AddTrackingData(const FString& Species, const FVector& Location)
{
    if (!bQuestActive || Species != TargetSpecies)
    {
        return;
    }

    FQuest_TrackingData NewTrack;
    NewTrack.DinosaurSpecies = Species;
    NewTrack.LastKnownLocation = Location;
    NewTrack.TrackingRadius = TrackingRange;
    NewTrack.TimeLastSeen = GetWorld()->GetTimeSeconds();
    NewTrack.bIsActiveTarget = true;

    ActiveTracks.Add(NewTrack);
    CompletedObservations++;

    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurTrackingSystem: Added tracking data for %s at location %s"), *Species, *Location.ToString());

    if (GEngine)
    {
        FString TrackMessage = FString::Printf(TEXT("Tracked %s! Progress: %d/%d"), *Species, CompletedObservations, RequiredObservations);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TrackMessage);
    }

    CheckTrackingProgress();
}

bool AQuest_DinosaurTrackingSystem::CheckTrackingProgress()
{
    if (!bQuestActive)
    {
        return false;
    }

    bool bObjectiveComplete = false;

    switch (CurrentObjective)
    {
        case EQuest_TrackingObjective::FindTracks:
        case EQuest_TrackingObjective::ObserveDinosaur:
        case EQuest_TrackingObjective::CountHerd:
            bObjectiveComplete = (CompletedObservations >= RequiredObservations);
            break;

        case EQuest_TrackingObjective::FollowTracks:
            bObjectiveComplete = (ActiveTracks.Num() >= 3 && CompletedObservations >= RequiredObservations);
            break;

        case EQuest_TrackingObjective::StudyBehavior:
            bObjectiveComplete = (CompletedObservations >= RequiredObservations && ActiveTracks.Num() >= 2);
            break;

        case EQuest_TrackingObjective::IdentifySpecies:
            bObjectiveComplete = (CompletedObservations >= RequiredObservations);
            break;
    }

    if (bObjectiveComplete)
    {
        CompleteTrackingObjective();
    }

    return bObjectiveComplete;
}

void AQuest_DinosaurTrackingSystem::CompleteTrackingObjective()
{
    if (!bQuestActive)
    {
        return;
    }

    bQuestActive = false;

    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurTrackingSystem: Tracking objective completed for %s"), *TargetSpecies);

    if (GEngine)
    {
        FString CompletionMessage = FString::Printf(TEXT("Quest Complete! Successfully tracked %s in %s biome"), *TargetSpecies, *CurrentBiome);
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, CompletionMessage);
    }

    // Reset for next quest
    ActiveTracks.Empty();
    CompletedObservations = 0;
}

FVector AQuest_DinosaurTrackingSystem::GetNearestTrackLocation(const FVector& PlayerLocation)
{
    if (ActiveTracks.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector NearestLocation = ActiveTracks[0].LastKnownLocation;
    float NearestDistance = FVector::Dist(PlayerLocation, NearestLocation);

    for (const FQuest_TrackingData& Track : ActiveTracks)
    {
        float Distance = FVector::Dist(PlayerLocation, Track.LastKnownLocation);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestLocation = Track.LastKnownLocation;
        }
    }

    return NearestLocation;
}

TArray<FString> AQuest_DinosaurTrackingSystem::GetAvailableSpeciesInBiome(const FString& BiomeName)
{
    TArray<FString> AvailableSpecies;

    if (BiomeName == TEXT("Savana"))
    {
        AvailableSpecies.Add(TEXT("TRex"));
        AvailableSpecies.Add(TEXT("Triceratops"));
        AvailableSpecies.Add(TEXT("Velociraptor"));
    }
    else if (BiomeName == TEXT("Floresta"))
    {
        AvailableSpecies.Add(TEXT("Brachiosaurus"));
        AvailableSpecies.Add(TEXT("Parasaurolophus"));
        AvailableSpecies.Add(TEXT("Velociraptor"));
    }
    else if (BiomeName == TEXT("Pantano"))
    {
        AvailableSpecies.Add(TEXT("Ankylosaurus"));
        AvailableSpecies.Add(TEXT("Triceratops"));
        AvailableSpecies.Add(TEXT("Protoceratops"));
    }
    else if (BiomeName == TEXT("Deserto"))
    {
        AvailableSpecies.Add(TEXT("Pachycephalo"));
        AvailableSpecies.Add(TEXT("Protoceratops"));
        AvailableSpecies.Add(TEXT("TRex"));
    }
    else if (BiomeName == TEXT("Montanha"))
    {
        AvailableSpecies.Add(TEXT("Tsintaosaurus"));
        AvailableSpecies.Add(TEXT("Ankylosaurus"));
        AvailableSpecies.Add(TEXT("Velociraptor"));
    }

    return AvailableSpecies;
}

FString AQuest_DinosaurTrackingSystem::DetectCurrentBiome(const FVector& Location)
{
    // Biome detection based on coordinates from global memory
    float X = Location.X;
    float Y = Location.Y;

    // Savana: (0, 0) ± 15000
    if (FMath::Abs(X) <= 15000.0f && FMath::Abs(Y) <= 15000.0f)
    {
        return TEXT("Savana");
    }
    // Pantano: (-50000, -45000) ± 15000
    else if (FMath::Abs(X + 50000.0f) <= 15000.0f && FMath::Abs(Y + 45000.0f) <= 15000.0f)
    {
        return TEXT("Pantano");
    }
    // Floresta: (-45000, 40000) ± 15000
    else if (FMath::Abs(X + 45000.0f) <= 15000.0f && FMath::Abs(Y - 40000.0f) <= 15000.0f)
    {
        return TEXT("Floresta");
    }
    // Deserto: (55000, 0) ± 15000
    else if (FMath::Abs(X - 55000.0f) <= 15000.0f && FMath::Abs(Y) <= 15000.0f)
    {
        return TEXT("Deserto");
    }
    // Montanha: (40000, 50000) ± 15000
    else if (FMath::Abs(X - 40000.0f) <= 15000.0f && FMath::Abs(Y - 50000.0f) <= 15000.0f)
    {
        return TEXT("Montanha");
    }

    return TEXT("Unknown");
}

void AQuest_DinosaurTrackingSystem::InitializeBiomeTrackingPoints()
{
    BiomeTrackingLocations.Empty();
    
    BiomeTrackingLocations.Add(TEXT("Savana"), FVector(0.0f, 0.0f, 100.0f));
    BiomeTrackingLocations.Add(TEXT("Pantano"), FVector(-50000.0f, -45000.0f, 100.0f));
    BiomeTrackingLocations.Add(TEXT("Floresta"), FVector(-45000.0f, 40000.0f, 100.0f));
    BiomeTrackingLocations.Add(TEXT("Deserto"), FVector(55000.0f, 0.0f, 100.0f));
    BiomeTrackingLocations.Add(TEXT("Montanha"), FVector(40000.0f, 50000.0f, 100.0f));

    UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurTrackingSystem: Initialized %d biome tracking points"), BiomeTrackingLocations.Num());
}

void AQuest_DinosaurTrackingSystem::OnTrackingZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bQuestActive || !OtherActor)
    {
        return;
    }

    // Check if it's a player character
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor))
    {
        if (PlayerCharacter->IsPlayerControlled())
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_DinosaurTrackingSystem: Player entered tracking zone"));
        }
    }
    // Check if it's a dinosaur
    else if (IsValidDinosaurActor(OtherActor))
    {
        FString Species = GetDinosaurSpeciesFromActor(OtherActor);
        if (Species == TargetSpecies)
        {
            AddTrackingData(Species, OtherActor->GetActorLocation());
        }
    }
}

void AQuest_DinosaurTrackingSystem::OnTrackingZoneOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (!bQuestActive || !OtherActor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Quest_DinosaurTrackingSystem: Actor %s left tracking zone"), *OtherActor->GetName());
}

void AQuest_DinosaurTrackingSystem::UpdateTrackingMarkers()
{
    if (!TrackingMarker)
    {
        return;
    }

    // Update marker visibility based on quest state
    TrackingMarker->SetVisibility(bQuestActive);
    
    if (bQuestActive && ActiveTracks.Num() > 0)
    {
        // Move marker to nearest track location
        FVector PlayerLocation = GetActorLocation();
        FVector NearestTrack = GetNearestTrackLocation(PlayerLocation);
        
        if (NearestTrack != FVector::ZeroVector)
        {
            TrackingMarker->SetWorldLocation(NearestTrack + FVector(0, 0, 200));
        }
    }
}

void AQuest_DinosaurTrackingSystem::ScanForDinosaurs()
{
    if (!bQuestActive)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get all actors in tracking range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (IsValidDinosaurActor(Actor))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= TrackingRange)
            {
                FString Species = GetDinosaurSpeciesFromActor(Actor);
                if (Species == TargetSpecies)
                {
                    // Check if we already have this track
                    bool bAlreadyTracked = false;
                    for (const FQuest_TrackingData& Track : ActiveTracks)
                    {
                        if (FVector::Dist(Track.LastKnownLocation, Actor->GetActorLocation()) < 500.0f)
                        {
                            bAlreadyTracked = true;
                            break;
                        }
                    }

                    if (!bAlreadyTracked)
                    {
                        AddTrackingData(Species, Actor->GetActorLocation());
                    }
                }
            }
        }
    }
}

void AQuest_DinosaurTrackingSystem::ProcessTrackingObjective()
{
    if (!bQuestActive)
    {
        return;
    }

    // Update tracking data timestamps
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (FQuest_TrackingData& Track : ActiveTracks)
    {
        // Mark tracks as inactive if they're too old (5 minutes)
        if (CurrentTime - Track.TimeLastSeen > 300.0f)
        {
            Track.bIsActiveTarget = false;
        }
    }

    // Remove inactive tracks
    ActiveTracks.RemoveAll([](const FQuest_TrackingData& Track) {
        return !Track.bIsActiveTarget;
    });
}

bool AQuest_DinosaurTrackingSystem::IsValidDinosaurActor(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }

    FString ActorName = Actor->GetName();
    
    // Check for dinosaur keywords in actor name
    return ActorName.Contains(TEXT("TRex")) || 
           ActorName.Contains(TEXT("Velociraptor")) || 
           ActorName.Contains(TEXT("Triceratops")) || 
           ActorName.Contains(TEXT("Brachiosaurus")) || 
           ActorName.Contains(TEXT("Ankylosaurus")) || 
           ActorName.Contains(TEXT("Parasaurolophus")) || 
           ActorName.Contains(TEXT("Pachycephalo")) || 
           ActorName.Contains(TEXT("Protoceratops")) || 
           ActorName.Contains(TEXT("Tsintaosaurus"));
}

FString AQuest_DinosaurTrackingSystem::GetDinosaurSpeciesFromActor(AActor* Actor)
{
    if (!Actor)
    {
        return TEXT("Unknown");
    }

    FString ActorName = Actor->GetName();
    
    if (ActorName.Contains(TEXT("TRex"))) return TEXT("TRex");
    if (ActorName.Contains(TEXT("Velociraptor"))) return TEXT("Velociraptor");
    if (ActorName.Contains(TEXT("Triceratops"))) return TEXT("Triceratops");
    if (ActorName.Contains(TEXT("Brachiosaurus"))) return TEXT("Brachiosaurus");
    if (ActorName.Contains(TEXT("Ankylosaurus"))) return TEXT("Ankylosaurus");
    if (ActorName.Contains(TEXT("Parasaurolophus"))) return TEXT("Parasaurolophus");
    if (ActorName.Contains(TEXT("Pachycephalo"))) return TEXT("Pachycephalo");
    if (ActorName.Contains(TEXT("Protoceratops"))) return TEXT("Protoceratops");
    if (ActorName.Contains(TEXT("Tsintaosaurus"))) return TEXT("Tsintaosaurus");
    
    return TEXT("Unknown");
}