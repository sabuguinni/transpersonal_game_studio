#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Anim_SurvivalMontageController.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Eating          UMETA(DisplayName = "Eating"),
    Resting         UMETA(DisplayName = "Resting"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Building        UMETA(DisplayName = "Building"),
    Climbing        UMETA(DisplayName = "Climbing")
};

USTRUCT(BlueprintType)
struct FAnim_SurvivalMontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    EAnim_SurvivalAction Action = EAnim_SurvivalAction::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    UAnimMontage* Montage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    float BlendInTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    float BlendOutTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Animation")
    bool bLooping = false;

    FAnim_SurvivalMontageData()
    {
        Action = EAnim_SurvivalAction::None;
        Montage = nullptr;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
        bLooping = false;
    }
};

/**
 * Controlador de montages para ações de sobrevivência
 * Gere a reprodução de animações específicas para ações como gathering, crafting, etc.
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalMontageController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalMontageController();

protected:
    virtual void BeginPlay() override;

    // Mapeamento de ações para montages
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Montages")
    TArray<FAnim_SurvivalMontageData> SurvivalMontages;

    // Componente de mesh para reproduzir animações
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    USkeletalMeshComponent* TargetMeshComponent;

    // Montage atualmente em reprodução
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    UAnimMontage* CurrentMontage;

    // Ação atual
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EAnim_SurvivalAction CurrentAction;

    // Timer para controlar duração das ações
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float ActionTimer;

    // Duração máxima para ações contínuas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MaxActionDuration;

public:
    // Reproduzir montage para uma ação específica
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    bool PlaySurvivalAction(EAnim_SurvivalAction Action, float Duration = 0.0f);

    // Parar a ação atual
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void StopCurrentAction();

    // Verificar se uma ação está em reprodução
    UFUNCTION(BlueprintPure, Category = "Survival Animation")
    bool IsPlayingAction(EAnim_SurvivalAction Action) const;

    // Obter a ação atual
    UFUNCTION(BlueprintPure, Category = "Survival Animation")
    EAnim_SurvivalAction GetCurrentAction() const { return CurrentAction; }

    // Configurar o componente de mesh alvo
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetTargetMeshComponent(USkeletalMeshComponent* MeshComponent);

    // Adicionar montage para uma ação
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void AddSurvivalMontage(EAnim_SurvivalAction Action, UAnimMontage* Montage, 
                           float PlayRate = 1.0f, bool bLooping = false);

    // Remover montage de uma ação
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void RemoveSurvivalMontage(EAnim_SurvivalAction Action);

    // Configurar parâmetros de blend para uma ação
    UFUNCTION(BlueprintCallable, Category = "Survival Animation")
    void SetActionBlendTimes(EAnim_SurvivalAction Action, float BlendIn, float BlendOut);

protected:
    // Encontrar dados do montage para uma ação
    FAnim_SurvivalMontageData* FindMontageData(EAnim_SurvivalAction Action);

    // Callback quando montage termina
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // Atualizar timer da ação
    void UpdateActionTimer(float DeltaTime);

    // Inicializar montages padrão
    void InitializeDefaultMontages();

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;
};