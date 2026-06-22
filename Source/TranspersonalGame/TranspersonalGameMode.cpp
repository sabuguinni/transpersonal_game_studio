// TranspersonalGameMode.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260622_007
// Sets DefaultPawnClass to TranspersonalCharacter so PIE spawns the player correctly

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set the default pawn class to our C++ character
    // This ensures PIE spawns TranspersonalCharacter at PlayerStart
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Use default player controller (can be replaced later with custom one)
    PlayerControllerClass = APlayerController::StaticClass();

    // No HUD yet — will be added in a later cycle
    HUDClass = nullptr;

    // Game state
    GameStateClass = AGameState::StaticClass();
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: BeginPlay — Prehistoric Survival Game started"));
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame — Map=%s"), *MapName);
}

APawn* ATranspersonalGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    // Spawn the character at the PlayerStart location
    APawn* SpawnedPawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
    if (SpawnedPawn)
    {
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Spawned pawn %s at %s"),
            *SpawnedPawn->GetName(),
            *StartSpot->GetActorLocation().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TranspersonalGameMode: Failed to spawn pawn for %s"),
            *NewPlayer->GetName());
    }
    return SpawnedPawn;
}

void ATranspersonalGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: RestartPlayer called"));
}
