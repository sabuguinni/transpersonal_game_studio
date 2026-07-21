// TranspersonalGameMode.cpp
// Transpersonal Game Studio — Engine Architect #02
// Prehistoric Survival Game — GameMode with DefaultPawnClass = TranspersonalCharacter

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "TranspersonalGameState.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set DefaultPawnClass to our ThirdPerson character
    // This is the CRITICAL binding that makes the player character spawn
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Set GameState class
    GameStateClass = ATranspersonalGameState::StaticClass();

    // Use default PlayerController (can be subclassed later)
    PlayerControllerClass = APlayerController::StaticClass();

    // No HUD yet — will be added in UI cycle
    HUDClass = nullptr;

    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] Initialized — DefaultPawnClass=TranspersonalCharacter"));
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] BeginPlay — Prehistoric survival world active"));
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] InitGame — Map: %s"), *MapName);
}

APawn* ATranspersonalGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    // Use parent implementation — spawns DefaultPawnClass at PlayerStart
    APawn* SpawnedPawn = Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
    if (SpawnedPawn)
    {
        UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] Spawned pawn: %s"), *SpawnedPawn->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[TranspersonalGameMode] Failed to spawn default pawn!"));
    }
    return SpawnedPawn;
}

void ATranspersonalGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] Player logged in: %s"), *NewPlayer->GetName());
}
