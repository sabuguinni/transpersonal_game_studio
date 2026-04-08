#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurHerdSystem.generated.h"

// Fragment para identificação única de dinossauros
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurIdentityFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID único do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DinosaurID = 0;

    // Espécie do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Species = NAME_None;

    // Características físicas únicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeVariation = 1.0f; // 0.8 a 1.2

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorVariation = FLinearColor::White;

    // Personalidade individual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggressiveness = 0.5f; // 0.0 a 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Curiosity = 0.5f; // 0.0 a 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sociability = 0.5f; // 0.0 a 1.0
};

// Fragment para comportamento de manada
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FHerdBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    // ID da manada (0 = solitário)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HerdID = 0;

    // Papel na manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 HerdRole = 0; // 0=Membro, 1=Líder, 2=Sentinela

    // Distância preferida do centro da manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredHerdDistance = 500.0f;

    // Força de coesão com a manada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionStrength = 1.0f;

    // Último dinossauro visto da mesma espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMassEntityHandle LastSeenHerdMember;

    // Tempo desde última interação social
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSinceLastSocialInteraction = 0.0f;
};

// Fragment para rotinas diárias
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDailyRoutineFragment : public FMassFragment
{
    GENERATED_BODY()

    // Atividade atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 CurrentActivity = 0; // 0=Pastar, 1=Beber, 2=Descansar, 3=Patrulhar, 4=Fugir

    // Tempo na atividade atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeInCurrentActivity = 0.0f;

    // Duração planejada da atividade
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlannedActivityDuration = 300.0f; // 5 minutos default

    // Localização de interesse atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    // Necessidades básicas (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Thirst = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fatigue = 0.3f;

    // Medo atual (aumenta com predadores próximos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fear = 0.0f;
};

// Processor principal para simulação de manadas
UCLASS()
class TRANSPERSONALGAME_API UDinosaurHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para dinossauros em manada
    FMassEntityQuery HerdMembersQuery;
    
    // Query para dinossauros solitários
    FMassEntityQuery SolitaryDinosaursQuery;

    // Processa comportamento de manada
    void ProcessHerdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Processa dinossauros solitários
    void ProcessSolitaryBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Calcula força de coesão da manada
    FVector CalculateHerdCohesion(const FTransformFragment& Transform, const FHerdBehaviorFragment& HerdBehavior, 
                                 FMassEntityManager& EntityManager);
    
    // Calcula força de separação (evitar colisões)
    FVector CalculateSeparation(const FTransformFragment& Transform, FMassEntityManager& EntityManager);
    
    // Calcula força de alinhamento com a manada
    FVector CalculateAlignment(const FHerdBehaviorFragment& HerdBehavior, FMassEntityManager& EntityManager);
};

// Processor para rotinas diárias
UCLASS()
class TRANSPERSONALGAME_API UDinosaurDailyRoutineProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurDailyRoutineProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery DailyRoutineQuery;
    
    // Atualiza necessidades básicas
    void UpdateBasicNeeds(FDailyRoutineFragment& Routine, float DeltaTime);
    
    // Decide próxima atividade baseada nas necessidades
    uint8 DecideNextActivity(const FDailyRoutineFragment& Routine, const FDinosaurIdentityFragment& Identity);
    
    // Encontra localização para atividade específica
    FVector FindActivityLocation(uint8 Activity, const FTransformFragment& Transform);
};

// Subsystem para gerenciar populações de dinossauros
UCLASS()
class TRANSPERSONALGAME_API UDinosaurPopulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Spawna uma manada de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Population")
    void SpawnHerd(FName Species, int32 HerdSize, FVector Location, float SpawnRadius = 1000.0f);

    // Spawna dinossauro solitário
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Population")
    void SpawnSolitaryDinosaur(FName Species, FVector Location);

    // Remove dinossauro (morte, despawn)
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Population")
    void RemoveDinosaur(FMassEntityHandle Entity);

    // Obtém estatísticas da população
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Population")
    int32 GetTotalDinosaurCount() const { return TotalDinosaurCount; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Population")
    int32 GetHerdCount() const { return ActiveHerds.Num(); }

protected:
    // Contador global de dinossauros
    UPROPERTY()
    int32 TotalDinosaurCount = 0;

    // Próximo ID único para dinossauros
    UPROPERTY()
    int32 NextDinosaurID = 1;

    // Próximo ID de manada
    UPROPERTY()
    int32 NextHerdID = 1;

    // Manadas ativas
    UPROPERTY()
    TMap<int32, TArray<FMassEntityHandle>> ActiveHerds;

    // Referência ao Mass Entity Subsystem
    UPROPERTY()
    class UMassEntitySubsystem* MassEntitySubsystem = nullptr;

private:
    // Gera características únicas para um dinossauro
    FDinosaurIdentityFragment GenerateUniqueIdentity(FName Species);
    
    // Gera variações físicas procedurais
    void GeneratePhysicalVariations(FDinosaurIdentityFragment& Identity);
    
    // Gera personalidade procedural
    void GeneratePersonality(FDinosaurIdentityFragment& Identity);
};