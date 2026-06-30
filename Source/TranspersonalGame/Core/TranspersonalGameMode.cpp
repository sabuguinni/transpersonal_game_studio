// TranspersonalGameMode.cpp
// Transpersonal Game Studio — Engine Architect #02
// Prehistoric survival game — sets DefaultPawnClass to TranspersonalCharacter

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set default pawn class to our TranspersonalCharacter
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Use default player controller
    PlayerControllerClass = APlayerController::StaticClass();

    // Use default HUD
    HUDClass = AHUD::StaticClass();

    // Spectator pawn class (fallback)
    SpectatorClass = ASpectatorPawn::StaticClass();

    // Game starts immediately
    bDelayedStart = false;
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: BeginPlay — Prehistoric Survival Active"));
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame — Map: %s"), *MapName);
}

void ATranspersonalGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (NewPlayer)
    {
        UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player logged in — %s"), *NewPlayer->GetName());
    }
}

void ATranspersonalGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player logged out"));
}
