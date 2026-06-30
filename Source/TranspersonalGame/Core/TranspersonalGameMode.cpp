// TranspersonalGameMode.cpp
// Engine Architect #02 — Cycle AUTO_20260630_003
// Sets DefaultPawnClass to TranspersonalCharacter for playable prototype

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set default pawn class to our playable character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
        TEXT("/Game/Blueprints/BP_TranspersonalCharacter"));

    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
    else
    {
        // Fallback to C++ class directly if Blueprint not found
        DefaultPawnClass = ATranspersonalCharacter::StaticClass();
    }

    // Use default player controller
    PlayerControllerClass = APlayerController::StaticClass();
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: BeginPlay — Prehistoric Survival World Active"));
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    UE_LOG(LogTemp, Log, TEXT("TranspersonalGameMode: InitGame on map '%s'"), *MapName);
}
