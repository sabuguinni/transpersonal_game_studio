// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — PROD_CYCLE_AUTO_20260702_005
// Manages build validation, module dependency checks, and integration health reporting.

#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Integration defaults
    bBuildStable = false;
    bContaminationClean = true;
    PlayabilityScore = 0;
    LoadedClassCount = 0;
    TotalActorCount = 0;
    CycleID = TEXT("PROD_CYCLE_AUTO_20260702_005");
    BuildStatus = TEXT("INITIALIZING");
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationValidation();
}

void ABuildIntegrationManager::RunIntegrationValidation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        BuildStatus = TEXT("FAIL_NO_WORLD");
        return;
    }

    // Count actors in world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    TotalActorCount = AllActors.Num();

    // Run contamination check
    bContaminationClean = RunContaminationCheck(AllActors);

    // Calculate playability score
    PlayabilityScore = CalculatePlayabilityScore(World, AllActors);

    // Mark build stable if all checks pass
    bBuildStable = bContaminationClean && (PlayabilityScore >= 60);
    BuildStatus = bBuildStable ? TEXT("STABLE") : TEXT("DEGRADED");

    UE_LOG(LogTemp, Log, TEXT("[BuildIntegrationManager] Cycle=%s Status=%s Score=%d Actors=%d Contamination=%s"),
        *CycleID, *BuildStatus, PlayabilityScore, TotalActorCount,
        bContaminationClean ? TEXT("CLEAN") : TEXT("CONTAMINATED"));
}

bool ABuildIntegrationManager::RunContaminationCheck(const TArray<AActor*>& Actors)
{
    // Forbidden keywords — spiritual/therapeutic content not allowed in this prehistoric survival game
    static const TArray<FString> ForbiddenKeywords = {
        TEXT("meditation"), TEXT("consciousness"), TEXT("spiritual"), TEXT("chakra"),
        TEXT("aura"), TEXT("mystic"), TEXT("transcend"), TEXT("enlighten"),
        TEXT("shaman"), TEXT("sacred"), TEXT("crystal"), TEXT("telepathy"),
        TEXT("awakening"), TEXT("energy_field"), TEXT("wisdom_keeper")
    };

    for (const AActor* Actor : Actors)
    {
        if (!Actor) continue;
        FString ActorLabel = Actor->GetActorLabel().ToLower();
        FString ActorName = Actor->GetName().ToLower();

        for (const FString& Keyword : ForbiddenKeywords)
        {
            if (ActorLabel.Contains(Keyword) || ActorName.Contains(Keyword))
            {
                UE_LOG(LogTemp, Warning, TEXT("[BuildIntegrationManager] CONTAMINATION: Actor '%s' contains forbidden keyword '%s'"),
                    *Actor->GetActorLabel(), *Keyword);
                return false;
            }
        }
    }

    return true;
}

int32 ABuildIntegrationManager::CalculatePlayabilityScore(UWorld* World, const TArray<AActor*>& Actors)
{
    if (!World) return 0;

    int32 Score = 0;

    // Check PlayerStart (20 pts)
    for (const AActor* Actor : Actors)
    {
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("PlayerStart")))
        {
            Score += 20;
            break;
        }
    }

    // Check dinosaur actors (20 pts)
    int32 DinoCount = 0;
    for (const AActor* Actor : Actors)
    {
        if (!Actor) continue;
        FString Label = Actor->GetActorLabel().ToLower();
        if (Label.Contains(TEXT("trex")) || Label.Contains(TEXT("raptor")) ||
            Label.Contains(TEXT("brach")) || Label.Contains(TEXT("dino")))
        {
            DinoCount++;
        }
    }
    Score += (DinoCount >= 3) ? 20 : (DinoCount > 0 ? 10 : 0);

    // Check directional light (20 pts)
    for (const AActor* Actor : Actors)
    {
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("DirectionalLight")))
        {
            Score += 20;
            break;
        }
    }

    // Check landscape (20 pts)
    for (const AActor* Actor : Actors)
    {
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Landscape")))
        {
            Score += 20;
            break;
        }
    }

    // Check NavMesh (20 pts)
    for (const AActor* Actor : Actors)
    {
        if (Actor && (Actor->GetClass()->GetName().Contains(TEXT("NavMesh")) ||
                      Actor->GetClass()->GetName().Contains(TEXT("NavBounds"))))
        {
            Score += 20;
            break;
        }
    }

    return Score;
}

FString ABuildIntegrationManager::GetBuildReport() const
{
    return FString::Printf(
        TEXT("BuildReport[Cycle=%s|Status=%s|Score=%d|Actors=%d|Classes=%d|Contamination=%s]"),
        *CycleID, *BuildStatus, PlayabilityScore, TotalActorCount, LoadedClassCount,
        bContaminationClean ? TEXT("CLEAN") : TEXT("CONTAMINATED")
    );
}
