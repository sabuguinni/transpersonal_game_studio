// TranspersonalGameMode.cpp
// Engine: Unreal Engine 5.5
// Studio: Transpersonal Game Studio
// Agent: #03 — Core Systems Programmer
// Purpose: Game mode for prehistoric survival — sets default pawn, controller, and HUD classes.

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set the default pawn class to our custom character
    // This ensures the player spawns as ATranspersonalCharacter at PlayerStart
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Use the default player controller (can be subclassed later for UI/input mapping)
    PlayerControllerClass = APlayerController::StaticClass();

    // HUD class left as default — custom HUD to be added by UI agent (#19)
    HUDClass = nullptr;

    // Spectator pawn class — default engine spectator
    SpectatorClass = nullptr;

    // Replication settings
    bStartPlayersAsSpectators = false;
    bDelayedStart = false;
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: BeginPlay — Prehistoric Survival World Active"));
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: DefaultPawnClass = %s"),
        DefaultPawnClass ? *DefaultPawnClass->GetName() : TEXT("None"));
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame — Map=%s"), *MapName);
}

APawn* ATranspersonalGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    // Delegate to parent — spawns ATranspersonalCharacter at PlayerStart
    return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

void ATranspersonalGameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);

    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: RestartPlayer — spawning character at PlayerStart"));
}
