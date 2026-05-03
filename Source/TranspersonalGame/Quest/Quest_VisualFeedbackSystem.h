#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TriggerBox.h"
#include "Quest_VisualFeedbackSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Water       UMETA(DisplayName = "Water Collection"),
    Shelter     UMETA(DisplayName = "Shelter Finding"),
    Observation UMETA(DisplayName = "Dinosaur Observation"),
    Resources   UMETA(DisplayName = "Resource Gathering"),
    Survival    UMETA(DisplayName = "General Survival")
};

UENUM(BlueprintType)
enum class EQuest_ProgressState : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ProgressState ProgressState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    FQuest_ObjectiveData()
    {
        ObjectiveName = TEXT("Default Objective");
        Description = TEXT("Complete this objective to progress");
        ObjectiveType = EQuest_ObjectiveType::Survival;
        ProgressState = EQuest_ProgressState::NotStarted;
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 200.0f;
        bIsOptional = false;
    }
};

/**
 * Sistema de feedback visual para quests de sobrevivência
 * Gere indicadores visuais, luzes de progresso e texto de objectivos
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_VisualFeedbackSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_VisualFeedbackSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes visuais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RewardMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* ProgressLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* ObjectiveTextComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTriggerBox* ProximityTrigger;

    // Dados do objectivo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    FQuest_ObjectiveData ObjectiveData;

    // Configurações visuais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    FLinearColor ObjectiveColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    float LightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    float TextSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    bool bShowProgressLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    bool bShowObjectiveText;

    // Estado do sistema
    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    bool bPlayerInProximity;

    UPROPERTY(BlueprintReadOnly, Category = "Quest State")
    float DistanceToPlayer;

public:
    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(EQuest_ProgressState NewState);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetObjectiveData(const FQuest_ObjectiveData& NewObjectiveData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ShowCompletionFeedback();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void HideVisualFeedback();

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsObjectiveCompleted() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    float GetCompletionPercentage() const;

protected:
    // Funções internas
    UFUNCTION()
    void OnProximityEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProximityExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateVisualComponents();
    void UpdateLightColor();
    void UpdateTextDisplay();
    void CalculatePlayerDistance();

    // Configurações por tipo de objectivo
    FLinearColor GetColorForObjectiveType(EQuest_ObjectiveType ObjectiveType) const;
    FString GetTextForObjectiveType(EQuest_ObjectiveType ObjectiveType) const;
    float GetLightIntensityForState(EQuest_ProgressState State) const;

private:
    // Cache do player
    UPROPERTY()
    APawn* CachedPlayer;

    // Timer para updates
    float UpdateTimer;
    static constexpr float UpdateInterval = 0.1f;
};