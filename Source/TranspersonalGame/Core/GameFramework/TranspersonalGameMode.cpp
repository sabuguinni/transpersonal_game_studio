// TranspersonalGameMode.cpp
// Prehistoric survival game — sets TranspersonalCharacter as default pawn
// Agent #03 — Core Systems Programmer — PROD_CYCLE_AUTO_20260627_009

#include "TranspersonalGameMode.h"
#include "Characters/TranspersonalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/HUD.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set the default pawn class to our playable character
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Use default player controller (handles input binding)
    PlayerControllerClass = APlayerController::StaticClass();

    // Use default HUD for now
    HUDClass = AHUD::StaticClass();

    unreal::log("TranspersonalGameMode: DefaultPawnClass = ATranspersonalCharacter");
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: BeginPlay — Prehistoric Survival World active"));
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame — Map=%s"), *MapName);
}
