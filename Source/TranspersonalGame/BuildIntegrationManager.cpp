// BuildIntegrationManager.cpp
// Integration & Build Agent #19 — PROD_CYCLE_AUTO_20260702_007
// Manages build integration, module health checks, and cross-agent validation.

#include "BuildIntegrationManager.h"
#include "TranspersonalGameState.h"
#include "TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bBuildHealthy = false;
    LoadedClassCount = 0;
    TotalClassCount = 7;
    LastValidationCycle = 0;
    IntegrationStatus = EBuild_IntegrationStatus::Pending;
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    RunIntegrationValidation();
}

void ABuildIntegrationManager::RunIntegrationValidation()
{
    LoadedClassCount = 0;
    ValidationErrors.Empty();

    // Validate world exists
    UWorld* World = GetWorld();
    if (!World)
    {
        ValidationErrors.Add(TEXT("World is null — cannot run integration validation"));
        IntegrationStatus = EBuild_IntegrationStatus::Failed;
        bBuildHealthy = false;
        return;
    }

    // Validate GameState
    ATranspersonalGameState* GameState = World->GetGameState<ATranspersonalGameState>();
    if (GameState)
    {
        LoadedClassCount++;
    }
    else
    {
        ValidationErrors.Add(TEXT("TranspersonalGameState not found in world"));
    }

    // Validate PlayerCharacter
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        ATranspersonalCharacter* Character = Cast<ATranspersonalCharacter>(PlayerPawn);
        if (Character)
        {
            LoadedClassCount++;
            ValidateCharacterStats(Character);
        }
        else
        {
            ValidationErrors.Add(TEXT("Player pawn is not a TranspersonalCharacter"));
        }
    }

    // Update status
    bBuildHealthy = (ValidationErrors.Num() == 0);
    IntegrationStatus = bBuildHealthy
        ? EBuild_IntegrationStatus::Healthy
        : EBuild_IntegrationStatus::Degraded;

    LastValidationCycle++;

    if (GEngine)
    {
        FString StatusMsg = FString::Printf(
            TEXT("[BuildIntegration] Cycle %d: %s — %d errors, %d/%d classes"),
            LastValidationCycle,
            bBuildHealthy ? TEXT("HEALTHY") : TEXT("DEGRADED"),
            ValidationErrors.Num(),
            LoadedClassCount,
            TotalClassCount
        );
        GEngine->AddOnScreenDebugMessage(-1, 10.0f,
            bBuildHealthy ? FColor::Green : FColor::Orange,
            StatusMsg);
    }
}

void ABuildIntegrationManager::ValidateCharacterStats(ATranspersonalCharacter* Character)
{
    if (!Character) return;

    // Verify survival stats are in valid range
    if (Character->Health < 0.0f || Character->Health > 100.0f)
    {
        ValidationErrors.Add(FString::Printf(
            TEXT("Character Health out of range: %.1f"), Character->Health));
    }
    if (Character->Hunger < 0.0f || Character->Hunger > 100.0f)
    {
        ValidationErrors.Add(FString::Printf(
            TEXT("Character Hunger out of range: %.1f"), Character->Hunger));
    }
    if (Character->Thirst < 0.0f || Character->Thirst > 100.0f)
    {
        ValidationErrors.Add(FString::Printf(
            TEXT("Character Thirst out of range: %.1f"), Character->Thirst));
    }
    if (Character->Stamina < 0.0f || Character->Stamina > 100.0f)
    {
        ValidationErrors.Add(FString::Printf(
            TEXT("Character Stamina out of range: %.1f"), Character->Stamina));
    }
}

bool ABuildIntegrationManager::IsBuildHealthy() const
{
    return bBuildHealthy;
}

int32 ABuildIntegrationManager::GetLoadedClassCount() const
{
    return LoadedClassCount;
}

TArray<FString> ABuildIntegrationManager::GetValidationErrors() const
{
    return ValidationErrors;
}

EBuild_IntegrationStatus ABuildIntegrationManager::GetIntegrationStatus() const
{
    return IntegrationStatus;
}
