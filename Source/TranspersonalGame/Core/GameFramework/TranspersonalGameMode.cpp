// TranspersonalGameMode.cpp
// Transpersonal Game Studio — Prehistoric Survival Game
// Agent #04 Performance Optimizer — PROD_CYCLE_AUTO_20260622_006

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "TranspersonalGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set the default pawn to our C++ character — no Blueprint dependency
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Use our custom game state
    GameStateClass = ATranspersonalGameState::StaticClass();

    // Survival mode defaults
    bSurvivalActive = true;
    RespawnDelay = 5.0f;
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] InitGame — Map: %s"), *MapName);
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] BeginPlay — Survival active: %s"), bSurvivalActive ? TEXT("true") : TEXT("false"));
}

void ATranspersonalGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] RestartPlayer called"));
}

bool ATranspersonalGameMode::IsSurvivalActive() const
{
    return bSurvivalActive;
}

void ATranspersonalGameMode::OnPlayerDied(AController* DeadPlayer)
{
    if (!DeadPlayer) return;
    UE_LOG(LogTemp, Warning, TEXT("[TranspersonalGameMode] Player died — scheduling respawn in %.1fs"), RespawnDelay);

    // Schedule respawn after delay
    FTimerHandle RespawnHandle;
    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindUFunction(this, FName("RestartPlayer"), DeadPlayer);
    GetWorldTimerManager().SetTimer(RespawnHandle, RespawnDelegate, RespawnDelay, false);
}
