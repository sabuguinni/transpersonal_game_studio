#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TranspersonalGame/Core/ConsciousnessComponent.h"
#include "TranspersonalGame/Audio/TranspersonalAudioManager.h"
#include "TranspersonalGame/Narrative/NarrativeStateManager.h"
#include "TranspersonalGameMode.generated.h"

UCLASS()
class TRANSPERSONALGAME_API ATranspersonalGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATranspersonalGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Game Mode Functions
    UFUNCTION(BlueprintCallable, Category = "Transpersonal Game")
    void InitializeTranspersonalSystems();

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Game")
    void RegisterPlayerConsciousness(UConsciousnessComponent* ConsciousnessComp);

    UFUNCTION(BlueprintCallable, Category = "Transpersonal Game")
    void TriggerGlobalConsciousnessEvent(const FGameplayTag& EventTag, float Intensity = 50.0f);

protected:
    // Subsystem References
    UPROPERTY()
    TObjectPtr<UTranspersonalAudioManager> AudioManager;

    UPROPERTY()
    TObjectPtr<UNarrativeStateManager> NarrativeManager;

    // Player Consciousness Tracking
    UPROPERTY()
    TWeakObjectPtr<UConsciousnessComponent> PlayerConsciousnessComponent;

    // Game State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    bool bEnableTranspersonalFeatures = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float GlobalConsciousnessMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    bool bDebugConsciousnessSystem = false;

    // Event Handlers
    UFUNCTION()
    void OnPlayerConsciousnessChanged(EConsciousnessState NewState, float Intensity);

    void InitializeAudioSystem();
    void InitializeNarrativeSystem();
    void SetupConsciousnessCallbacks();
};