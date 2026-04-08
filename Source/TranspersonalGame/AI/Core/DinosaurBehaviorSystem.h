#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/ActorComponent.h"
#include "DinosaurBehaviorSystem.generated.h"

class ADinosaurCharacter;
class UDinosaurPersonality;
class UDinosaurMemoryComponent;

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros Pequenos (Domesticáveis)
    Compsognathus,
    Parasaurolophus,
    Triceratops_Juvenile,
    
    // Herbívoros Grandes (Neutros)
    Triceratops_Adult,
    Brachiosaurus,
    Stegosaurus,
    
    // Carnívoros Pequenos (Ameaça Baixa)
    Deinonychus,
    Dilophosaurus,
    
    // Carnívoros Médios (Ameaça Alta)
    Allosaurus,
    Carnotaurus,
    
    // Carnívoros Grandes (Ameaça Extrema)
    Tyrannosaurus,
    Spinosaurus,
    Giganotosaurus
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Foraging,
    Hunting,
    Drinking,
    Resting,
    Socializing,
    Fleeing,
    Territorial,
    Mating,
    Nesting,
    Migrating,
    Investigating,
    Stalking,
    Ambushing
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive,      // Herbívoros pequenos
    Cautious,     // Herbívoros grandes
    Opportunist,  // Carnívoros pequenos
    Aggressive,   // Carnívoros médios
    Apex          // Carnívoros grandes
};

USTRUCT(BlueprintType)
struct FDinosaurPersonalityTraits
{
    GENERATED_BODY()

    // Traços de personalidade (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Territoriality = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0, ClampMax = 1.0))
    float Intelligence = 0.5f;

    // Variações físicas únicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SizeModifier = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorVariation = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoicePitch = 1.0f;

    FDinosaurPersonalityTraits()
    {
        // Valores padrão já definidos acima
    }
};

USTRUCT(BlueprintType)
struct FDinosaurRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState PrimaryActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 0.0f; // Hora do dia (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 2.0f; // Duração em horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;
};

/**
 * Sistema central de comportamento para dinossauros
 * Gere rotinas diárias, personalidade, memória e interações sociais
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO DA ESPÉCIE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bCanBeDomesticated = false;

    // === PERSONALIDADE ÚNICA ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FDinosaurPersonalityTraits PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString IndividualName;

    // === ROTINAS DIÁRIAS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routines")
    TArray<FDinosaurRoutine> DailyRoutines;

    // === ESTADO ATUAL ===
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDinosaurBehaviorState CurrentBehaviorState = EDinosaurBehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentStateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget = nullptr;

    // === NECESSIDADES BÁSICAS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = 0.0, ClampMax = 100.0))
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = 0.0, ClampMax = 100.0))
    float Thirst = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = 0.0, ClampMax = 100.0))
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = 0.0, ClampMax = 100.0))
    float Social = 50.0f;

    // === DOMESTICAÇÃO ===
    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    float DomesticationLevel = 0.0f; // 0-100

    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    AActor* TrustedHuman = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float TrustDecayRate = 1.0f; // Por dia sem interação positiva

    // === FUNÇÕES PÚBLICAS ===
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState, AActor* Target = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddRoutine(const FDinosaurRoutine& NewRoutine);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FDinosaurRoutine GetActiveRoutine() const;

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void AddTrust(float Amount, AActor* Human);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void RemoveTrust(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanBeApproached(AActor* Human) const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void ModifyNeed(const FString& NeedName, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void GenerateUniquePersonality();

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityTrait(const FString& TraitName) const;

protected:
    // === FUNÇÕES INTERNAS ===
    void UpdateNeeds(float DeltaTime);
    void UpdateRoutines(float DeltaTime);
    void UpdateDomestication(float DeltaTime);
    void EvaluateBehaviorChange();
    
    FDinosaurRoutine* FindActiveRoutine();
    float GetCurrentTimeOfDay() const;
    
    // === COMPONENTES REQUERIDOS ===
    UPROPERTY()
    class UDinosaurMemoryComponent* MemoryComponent;

    UPROPERTY()
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;

private:
    float LastNeedsUpdate = 0.0f;
    float LastRoutineCheck = 0.0f;
    bool bPersonalityGenerated = false;
};