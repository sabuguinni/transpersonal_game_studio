// TranspersonalGameMode.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260622_008
// Sets DefaultPawnClass to TranspersonalCharacter and configures game rules.

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "TranspersonalGameState.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set the default pawn class to our TranspersonalCharacter
    // Blueprint override BP_TranspersonalCharacter takes priority if it exists
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
        TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));

    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
    else
    {
        DefaultPawnClass = ATranspersonalCharacter::StaticClass();
    }

    // Use our custom game state
    GameStateClass = ATranspersonalGameState::StaticClass();

    // Use default player controller (can be subclassed later)
    PlayerControllerClass = APlayerController::StaticClass();

    // Game rules
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
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame — Map=%s"), *MapName);
}

void ATranspersonalGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player logged in — %s"),
        NewPlayer ? *NewPlayer->GetName() : TEXT("NULL"));
}

void ATranspersonalGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: Player logged out"));
}
