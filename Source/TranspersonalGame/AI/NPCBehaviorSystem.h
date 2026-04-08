#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ego            UMETA(DisplayName = "Ego State"),
    Shadow         UMETA(DisplayName = "Shadow Integration"),
    Anima          UMETA(DisplayName = "Anima/Animus"),
    Wise           UMETA(DisplayName = "Wise Self"),
    Transcendent   UMETA(DisplayName = "Transcendent Unity")
};

UENUM(BlueprintType)
enum class ENPCArchetype : uint8
{
    ShadowSelf     UMETA(DisplayName = "Shadow Self"),
    WiseGuide      UMETA(DisplayName = "Wise Guide"),
    InnerChild     UMETA(DisplayName = "Inner Child"),
    Anima          UMETA(DisplayName = "Anima"),
    Animus         UMETA(DisplayName = "Animus"),
    Trickster      UMETA(DisplayName = "Trickster"),
    GreatMother    UMETA(DisplayName = "Great Mother"),
    Senex          UMETA(DisplayName = "Senex")
};

USTRUCT(BlueprintType)
struct FNPCBehaviorData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState PreferredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessInfluenceRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransformationSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DialogueKeys;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> BehaviorAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanInitiateTransformation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalResonance = 0.5f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCArchetype NPCArchetype = ENPCArchetype::ShadowSelf;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    EConsciousnessState CurrentConsciousnessState = EConsciousnessState::Ego;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float ConsciousnessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float EmotionalState = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    bool bIsTransforming = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    class UDataTable* NPCBehaviorDataTable;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeNPCBehavior(ENPCArchetype InArchetype);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void TriggerTransformation();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RespondToPlayerConsciousness(float PlayerConsciousnessLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Behavior")
    void OnConsciousnessStateChanged(EConsciousnessState OldState, EConsciousnessState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Behavior")
    void OnTransformationTriggered();

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC Behavior")
    void OnEmotionalResonance(float ResonanceLevel);

private:
    FNPCBehaviorData* CurrentBehaviorData;
    float TransformationTimer = 0.0f;
    
    void UpdateBehaviorBasedOnConsciousness();
    void CheckPlayerProximity();
    float CalculateEmotionalResonance(class ACharacter* Player);
};