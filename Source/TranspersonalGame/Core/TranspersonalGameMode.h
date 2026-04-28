#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TranspersonalGameMode.generated.h"

/**
 * Main Game Mode for Transpersonal Dinosaur Survival Game
 * Sets default pawn, player controller, and HUD classes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATranspersonalGameMode();

protected:
    virtual void BeginPlay() override;

public:
    /** Initialize core game systems */
    UFUNCTION(BlueprintCallable, Category = "Game")
    void InitializeGameSystems();
};
