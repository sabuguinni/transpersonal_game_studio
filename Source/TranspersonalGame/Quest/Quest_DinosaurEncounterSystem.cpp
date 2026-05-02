#include "Quest_DinosaurEncounterSystem.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AQuest_DinosaurEncounterSystem::AQuest_DinosaurEncounterSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(1000.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    EncounterCheckInterval = 2.0f;
    MaxSimultaneousEncounters = 3;
    bDebugMode = true;
    bQuestEncounterActive = false;
    CurrentQuestObjective = TEXT("Explore the prehistoric world");
    LastEncounterCheckTime = 0.0f;
    ActiveEncounterCount = 0;

    // Create default encounters
    ActiveEncounters.Empty();
    
    // Peaceful Brachiosaurus encounter
    FQuest_DinosaurEncounter BrachioEncounter;
    BrachioEncounter.Species = EQuest_DinosaurSpecies::Brachiosaurus;
    BrachioEncounter.EncounterType = EQuest_EncounterType::Feeding;
    BrachioEncounter.Location = FVector(2000.0f, 1000.0f, 100.0f);
    BrachioEncounter.TriggerRadius = 800.0f;
    BrachioEncounter.EncounterDescription = TEXT("Observe the massive Brachiosaurus feeding on tall trees");
    BrachioEncounter.bIsActive = true;
    ActiveEncounters.Add(BrachioEncounter);

    // Territorial Raptor pack
    FQuest_DinosaurEncounter RaptorEncounter;
    RaptorEncounter.Species = EQuest_DinosaurSpecies::Raptor;
    RaptorEncounter.EncounterType = EQuest_EncounterType::Territorial;
    RaptorEncounter.Location = FVector(-1500.0f, 800.0f, 50.0f);
    RaptorEncounter.TriggerRadius = 600.0f;
    RaptorEncounter.EncounterDescription = TEXT("Avoid the territorial Velociraptor pack");
    RaptorEncounter.bIsActive = true;
    ActiveEncounters.Add(RaptorEncounter);

    // Migrating Triceratops herd
    FQuest_DinosaurEncounter TriceratopsEncounter;
    TriceratopsEncounter.Species = EQuest_DinosaurSpecies::Triceratops;
    TriceratopsEncounter.EncounterType = EQuest_EncounterType::Migration;
    TriceratopsEncounter.Location = FVector(500.0f, -2000.0f, 80.0f);
    TriceratopsEncounter.TriggerRadius = 700.0f;
    TriceratopsEncounter.EncounterDescription = TEXT("Follow the migrating Triceratops herd");
    TriceratopsEncounter.bIsActive = true;
    ActiveEncounters.Add(TriceratopsEncounter);
}

void AQuest_DinosaurEncounterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Started with %d encounters"), ActiveEncounters.Num());
    }

    // Initialize all encounters
    for (int32 i = 0; i < ActiveEncounters.Num(); ++i)
    {
        if (ActiveEncounters[i].bIsActive)
        {
            SpawnDinosaurAtLocation(ActiveEncounters[i].Species, ActiveEncounters[i].Location);
        }
    }
}

void AQuest_DinosaurEncounterSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check for encounters at regular intervals
    if (GetWorld()->GetTimeSeconds() - LastEncounterCheckTime > EncounterCheckInterval)
    {
        CheckForPlayerEncounters();
        LastEncounterCheckTime = GetWorld()->GetTimeSeconds();
    }
}

void AQuest_DinosaurEncounterSystem::CreateDinosaurEncounter(EQuest_DinosaurSpecies Species, EQuest_EncounterType Type, FVector Location)
{
    if (ActiveEncounters.Num() >= MaxSimultaneousEncounters)
    {
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Maximum encounters reached"));
        }
        return;
    }

    FQuest_DinosaurEncounter NewEncounter;
    NewEncounter.Species = Species;
    NewEncounter.EncounterType = Type;
    NewEncounter.Location = Location;
    NewEncounter.TriggerRadius = 500.0f;
    NewEncounter.EncounterDescription = FString::Printf(TEXT("Encounter with %s - %s"), 
        *GetSpeciesName(Species), *GetEncounterTypeName(Type));
    NewEncounter.bIsActive = true;

    ActiveEncounters.Add(NewEncounter);
    SpawnDinosaurAtLocation(Species, Location);

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Created %s encounter at %s"), 
            *GetSpeciesName(Species), *Location.ToString());
    }
}

void AQuest_DinosaurEncounterSystem::TriggerEncounter(int32 EncounterIndex)
{
    if (!ActiveEncounters.IsValidIndex(EncounterIndex))
    {
        return;
    }

    const FQuest_DinosaurEncounter& Encounter = ActiveEncounters[EncounterIndex];
    
    if (!Encounter.bIsActive)
    {
        return;
    }

    // Broadcast encounter event
    OnEncounterTriggered.Broadcast(Encounter.Species, Encounter.EncounterType);

    // Play encounter audio
    PlayEncounterAudio(Encounter.Species, Encounter.EncounterType);

    // Update quest objective
    FString ObjectiveText = FString::Printf(TEXT("Survive the %s encounter"), *GetSpeciesName(Encounter.Species));
    UpdateQuestObjective(ObjectiveText);

    // Display encounter message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("ENCOUNTER: %s"), *Encounter.EncounterDescription));
    }

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Triggered encounter %d - %s"), 
            EncounterIndex, *Encounter.EncounterDescription);
    }

    bQuestEncounterActive = true;
}

void AQuest_DinosaurEncounterSystem::CompleteEncounter(int32 EncounterIndex)
{
    if (!ActiveEncounters.IsValidIndex(EncounterIndex))
    {
        return;
    }

    ActiveEncounters[EncounterIndex].bIsActive = false;
    bQuestEncounterActive = false;

    // Update quest objective
    UpdateQuestObjective(TEXT("Continue exploring the prehistoric world"));

    // Display completion message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
            TEXT("Encounter completed successfully!"));
    }

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Completed encounter %d"), EncounterIndex);
    }
}

bool AQuest_DinosaurEncounterSystem::IsPlayerInEncounterRange(const FQuest_DinosaurEncounter& Encounter)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Encounter.Location);
    return Distance <= Encounter.TriggerRadius;
}

void AQuest_DinosaurEncounterSystem::UpdateQuestObjective(const FString& NewObjective)
{
    CurrentQuestObjective = NewObjective;
    OnObjectiveUpdated.Broadcast(NewObjective);

    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Updated objective - %s"), *NewObjective);
    }
}

void AQuest_DinosaurEncounterSystem::PlayEncounterAudio(EQuest_DinosaurSpecies Species, EQuest_EncounterType Type)
{
    // Audio implementation would go here
    // For now, just log the audio that should be played
    FString AudioDescription = FString::Printf(TEXT("%s %s audio"), 
        *GetSpeciesName(Species), *GetEncounterTypeName(Type));
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Playing audio - %s"), *AudioDescription);
    }
}

void AQuest_DinosaurEncounterSystem::CheckForPlayerEncounters()
{
    for (int32 i = 0; i < ActiveEncounters.Num(); ++i)
    {
        if (ActiveEncounters[i].bIsActive && IsPlayerInEncounterRange(ActiveEncounters[i]))
        {
            TriggerEncounter(i);
        }
    }
}

void AQuest_DinosaurEncounterSystem::SpawnDinosaurAtLocation(EQuest_DinosaurSpecies Species, FVector Location)
{
    // For now, we'll use basic shapes as placeholders
    // In a full implementation, this would spawn actual dinosaur actors
    
    if (bDebugMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurEncounterSystem: Spawning %s at %s"), 
            *GetSpeciesName(Species), *Location.ToString());
    }
}

FString AQuest_DinosaurEncounterSystem::GetSpeciesName(EQuest_DinosaurSpecies Species)
{
    switch (Species)
    {
        case EQuest_DinosaurSpecies::TRex:
            return TEXT("Tyrannosaurus Rex");
        case EQuest_DinosaurSpecies::Raptor:
            return TEXT("Velociraptor");
        case EQuest_DinosaurSpecies::Triceratops:
            return TEXT("Triceratops");
        case EQuest_DinosaurSpecies::Brachiosaurus:
            return TEXT("Brachiosaurus");
        case EQuest_DinosaurSpecies::Stegosaurus:
            return TEXT("Stegosaurus");
        default:
            return TEXT("Unknown Species");
    }
}

FString AQuest_DinosaurEncounterSystem::GetEncounterTypeName(EQuest_EncounterType Type)
{
    switch (Type)
    {
        case EQuest_EncounterType::Peaceful:
            return TEXT("Peaceful");
        case EQuest_EncounterType::Territorial:
            return TEXT("Territorial");
        case EQuest_EncounterType::Hunting:
            return TEXT("Hunting");
        case EQuest_EncounterType::Feeding:
            return TEXT("Feeding");
        case EQuest_EncounterType::Migration:
            return TEXT("Migration");
        default:
            return TEXT("Unknown Type");
    }
}