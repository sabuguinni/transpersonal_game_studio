// TranspersonalGameMode.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Prehistoric survival game — sets up default pawn, player controller, game state

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "TranspersonalGameState.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set default pawn to our custom character so the player spawns correctly
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Use the built-in PlayerController — custom controller can be added later
    PlayerControllerClass = APlayerController::StaticClass();

    // Use our custom GameState
    GameStateClass = ATranspersonalGameState::StaticClass();

    // HUD class — null for now, UI agent will set this
    HUDClass = nullptr;
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: BeginPlay — prehistoric survival world initialized"));
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame — Map=%s"), *MapName);
}

APawn* ATranspersonalGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}
