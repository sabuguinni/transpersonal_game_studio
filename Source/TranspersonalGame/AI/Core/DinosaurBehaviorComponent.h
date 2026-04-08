#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurBehaviorTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorComponent.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBehaviorStateChanged, EDinosaurBehaviorState, OldState, EDinosaurBehaviorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDomesticationProgress, float, TrustLevel);

/**
 * Componente central que gere o comportamento individual de cada dinossauro
 * Integra-se com Behavior Trees mas mantém lógica de alto nível
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuração da espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FDataTableRowHandle SpeciesProfile;

    // Estado comportamental atual
    UPROPERTY(BlueprintReadOnly, Category = "Behavior")
    EDinosaurBehaviorState CurrentBehaviorState = EDinosaurBehaviorState::Idle;

    // Necessidades atuais (0-1, onde 1 = necessidade crítica)
    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    TMap<EDinosaurNeed, float> CurrentNeeds;

    // Sistema de memória individual
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FDinosaurMemory IndividualMemory;

    // Personalidade individual (varia entre indivíduos da mesma espécie)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float IndividualAggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float IndividualCuriosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float IndividualSocialness = 0.5f;

    // Sistema de domesticação
    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    bool bIsDomesticated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    bool bIsBeingDomesticated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    float DomesticationProgress = 0.0f;

    // Eventos
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBehaviorStateChanged OnBehaviorStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDomesticationProgress OnDomesticationProgress;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetNeedLevel(EDinosaurNeed Need) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ModifyNeed(EDinosaurNeed Need, float Delta);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberLocation(FGameplayTag LocationTag, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetKnownLocation(FGameplayTag LocationTag) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberDinosaur(int32 DinosaurID, float Relationship);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void StartDomesticationProcess();

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void AddPlayerInteraction(bool bPositive);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanBeDomesticated() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FDinosaurDailyRoutine GetCurrentPreferredRoutine() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldReactToPlayer(float DistanceToPlayer) const;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurBehaviorState GetPlayerReactionState(float DistanceToPlayer) const;

protected:
    // Referências aos componentes de IA
    UPROPERTY()
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComponent;

    // Timers internos
    float LastRoutineCheck = 0.0f;
    float LastNeedUpdate = 0.0f;
    float LastMemoryUpdate = 0.0f;

    // Cache do perfil da espécie
    const FDinosaurSpeciesProfile* CachedSpeciesProfile = nullptr;

    // Funções internas
    void UpdateNeeds(float DeltaTime);
    void UpdateRoutines(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void UpdateDomestication(float DeltaTime);
    void UpdateBlackboardValues();

    const FDinosaurSpeciesProfile* GetSpeciesProfile() const;
    float GetCurrentTimeOfDay() const;
    bool IsInPreferredTimeRange(const FDinosaurDailyRoutine& Routine) const;
    float CalculateRoutinePriority(const FDinosaurDailyRoutine& Routine) const;
};