// TranspersonalGameMode.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Wires DefaultPawnClass = ATranspersonalCharacter, integrates DinosaurBase damage pipeline.

#include "TranspersonalGameMode.h"
#include "TranspersonalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/HUD.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    // Set the default pawn to our prehistoric survivor character
    DefaultPawnClass = ATranspersonalCharacter::StaticClass();

    // Use default HUD for now — UI agent will replace this
    HUDClass = AHUD::StaticClass();

    // Use default player controller
    PlayerControllerClass = APlayerController::StaticClass();

    // Game starts immediately — no lobby/menu in MVP
    bStartPlayersAsSpectators = false;

    // Default game state
    GameStateClass = AGameStateBase::StaticClass();
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] BeginPlay — Prehistoric Survival World initialised."));
    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] DefaultPawnClass = %s"), *DefaultPawnClass->GetName());
}

void ATranspersonalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] InitGame — Map: %s"), *MapName);
}

void ATranspersonalGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer && NewPlayer->GetPawn())
    {
        UE_LOG(LogTemp, Log, TEXT("[TranspersonalGameMode] Player logged in — Pawn: %s"),
            *NewPlayer->GetPawn()->GetName());
    }
}

float ATranspersonalGameMode::ModifyDamage(float BaseDamage, AActor* DamagedActor,
    AActor* DamageCauser, AController* InstigatedBy,
    TSubclassOf<UDamageType> DamageTypeClass) const
{
    if (!DamagedActor)
    {
        return BaseDamage;
    }

    // Difficulty scaling — can be exposed as UPROPERTY later
    const float DifficultyMultiplier = 1.0f;

    // Apply global difficulty modifier
    float FinalDamage = BaseDamage * DifficultyMultiplier;

    UE_LOG(LogTemp, Verbose, TEXT("[TranspersonalGameMode] ModifyDamage: %.1f -> %.1f on %s"),
        BaseDamage, FinalDamage, *DamagedActor->GetName());

    return FinalDamage;
}
