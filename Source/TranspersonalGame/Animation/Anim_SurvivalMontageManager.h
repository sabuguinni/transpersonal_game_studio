#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "Anim_SurvivalMontageManager.generated.h"

/**
 * Tipos de acções de sobrevivência que têm montagens específicas
 */
UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering Resources"),
    Crafting        UMETA(DisplayName = "Crafting Tools"),
    Drinking        UMETA(DisplayName = "Drinking Water"),
    Eating          UMETA(DisplayName = "Eating Food"),
    Building        UMETA(DisplayName = "Building Shelter"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Cooking         UMETA(DisplayName = "Cooking Food"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Hiding          UMETA(DisplayName = "Hiding from Predators")
};

/**
 * Dados de configuração para uma montagem de sobrevivência
 */
USTRUCT(BlueprintType)
struct FAnim_SurvivalMontageData
{
    GENERATED_BODY()

    /** Montagem de animação a reproduzir */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> Montage;

    /** Duração base da acção (pode ser modificada por factores externos) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.1"))
    float BaseDuration = 2.0f;

    /** Velocidade de reprodução da animação */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.1"))
    float PlayRate = 1.0f;

    /** Se verdadeiro, a acção pode ser interrompida */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bCanBeInterrupted = true;

    /** Tempo de blend in para a animação */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0"))
    float BlendInTime = 0.2f;

    /** Tempo de blend out para a animação */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0"))
    float BlendOutTime = 0.2f;

    FAnim_SurvivalMontageData()
    {
        Montage = nullptr;
        BaseDuration = 2.0f;
        PlayRate = 1.0f;
        bCanBeInterrupted = true;
        BlendInTime = 0.2f;
        BlendOutTime = 0.2f;
    }
};

/**
 * Gestor de montagens de animação para acções de sobrevivência
 * Controla a reprodução de animações específicas para diferentes acções do jogador
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_SurvivalMontageManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalMontageManager();

protected:
    virtual void BeginPlay() override;

public:
    /** Mapa de montagens por tipo de acção */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Montages")
    TMap<EAnim_SurvivalAction, FAnim_SurvivalMontageData> SurvivalMontages;

    /** Acção actualmente a ser executada */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAnim_SurvivalAction CurrentAction = EAnim_SurvivalAction::None;

    /** Se verdadeiro, uma acção está actualmente a ser executada */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPerformingAction = false;

    /** Tempo restante da acção actual */
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float RemainingActionTime = 0.0f;

    /** Referência para a instância de animação do personagem */
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    UAnimInstance* AnimInstance = nullptr;

public:
    /**
     * Iniciar uma acção de sobrevivência
     * @param Action Tipo de acção a executar
     * @param DurationMultiplier Multiplicador para a duração base da acção
     * @return true se a acção foi iniciada com sucesso
     */
    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool StartSurvivalAction(EAnim_SurvivalAction Action, float DurationMultiplier = 1.0f);

    /**
     * Interromper a acção actual se possível
     * @return true se a acção foi interrompida
     */
    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool InterruptCurrentAction();

    /**
     * Verificar se uma acção específica está disponível
     * @param Action Tipo de acção a verificar
     * @return true se a acção tem montagem configurada
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival Actions")
    bool IsActionAvailable(EAnim_SurvivalAction Action) const;

    /**
     * Obter a duração estimada de uma acção
     * @param Action Tipo de acção
     * @param DurationMultiplier Multiplicador a aplicar
     * @return Duração em segundos
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival Actions")
    float GetActionDuration(EAnim_SurvivalAction Action, float DurationMultiplier = 1.0f) const;

    /**
     * Configurar uma montagem para um tipo de acção
     * @param Action Tipo de acção
     * @param MontageData Dados da montagem a configurar
     */
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetMontageForAction(EAnim_SurvivalAction Action, const FAnim_SurvivalMontageData& MontageData);

    /**
     * Event chamado quando uma acção é iniciada
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnActionStarted(EAnim_SurvivalAction Action);

    /**
     * Event chamado quando uma acção é completada
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnActionCompleted(EAnim_SurvivalAction Action);

    /**
     * Event chamado quando uma acção é interrompida
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnActionInterrupted(EAnim_SurvivalAction Action);

protected:
    /**
     * Inicializar montagens padrão
     */
    void InitializeDefaultMontages();

    /**
     * Encontrar e configurar a instância de animação
     */
    void SetupAnimInstance();

    /**
     * Callback chamado quando uma montagem termina
     */
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    /**
     * Actualizar o tempo restante da acção
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    /** Timer handle para controlar a duração das acções */
    FTimerHandle ActionTimerHandle;

    /** Montagem actualmente a ser reproduzida */
    UAnimMontage* CurrentMontage = nullptr;

    /** Finalizar a acção actual */
    void FinishCurrentAction();
};