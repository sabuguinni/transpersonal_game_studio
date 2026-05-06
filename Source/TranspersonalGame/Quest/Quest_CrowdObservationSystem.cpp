#include "Quest_CrowdObservationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"

AQuest_CrowdObservationSystem::AQuest_CrowdObservationSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Criar componente de detecção
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    RootComponent = DetectionSphere;
    DetectionSphere->SetSphereRadius(2000.0f);
    
    // Configurar mesh visual
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    
    // Configurações iniciais
    ObservationRadius = 2000.0f;
    MinCrowdSize = 3;
    ObservationTime = 10.0f;
    CurrentObservationTime = 0.0f;
    bIsObserving = false;
    bQuestCompleted = false;
    
    // Configurar delegates
    DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_CrowdObservationSystem::OnPlayerEnterObservationZone);
    DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_CrowdObservationSystem::OnPlayerExitObservationZone);
}

void AQuest_CrowdObservationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdObservationSystem: Sistema iniciado"));
    
    // Configurar detecção apenas para o jogador
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AQuest_CrowdObservationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsObserving && !bQuestCompleted)
    {
        UpdateObservation(DeltaTime);
    }
}

void AQuest_CrowdObservationSystem::OnPlayerEnterObservationZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsPlayerCharacter(OtherActor))
    {
        StartObservation();
    }
}

void AQuest_CrowdObservationSystem::OnPlayerExitObservationZone(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (IsPlayerCharacter(OtherActor))
    {
        StopObservation();
    }
}

void AQuest_CrowdObservationSystem::StartObservation()
{
    if (bQuestCompleted)
        return;
        
    // Verificar se há dinossauros suficientes na área
    TArray<AActor*> NearbyDinosaurs = FindNearbyDinosaurs();
    
    if (NearbyDinosaurs.Num() >= MinCrowdSize)
    {
        bIsObserving = true;
        CurrentObservationTime = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdObservationSystem: Observação iniciada - %d dinossauros detectados"), NearbyDinosaurs.Num());
        
        // Notificar o jogador
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                    FString::Printf(TEXT("Observação iniciada: %d dinossauros detectados. Observe por %.0f segundos."), 
                    NearbyDinosaurs.Num(), ObservationTime));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdObservationSystem: Dinossauros insuficientes - %d encontrados, %d necessários"), NearbyDinosaurs.Num(), MinCrowdSize);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("Dinossauros insuficientes na área: %d/%d"), NearbyDinosaurs.Num(), MinCrowdSize));
        }
    }
}

void AQuest_CrowdObservationSystem::StopObservation()
{
    if (bIsObserving)
    {
        bIsObserving = false;
        CurrentObservationTime = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdObservationSystem: Observação interrompida"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Observação interrompida - você saiu da área"));
        }
    }
}

void AQuest_CrowdObservationSystem::UpdateObservation(float DeltaTime)
{
    CurrentObservationTime += DeltaTime;
    
    // Verificar se ainda há dinossauros suficientes
    TArray<AActor*> NearbyDinosaurs = FindNearbyDinosaurs();
    
    if (NearbyDinosaurs.Num() < MinCrowdSize)
    {
        StopObservation();
        return;
    }
    
    // Mostrar progresso
    float Progress = (CurrentObservationTime / ObservationTime) * 100.0f;
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Cyan, 
            FString::Printf(TEXT("Observação: %.1f%% completa"), Progress));
    }
    
    // Verificar se a observação foi completada
    if (CurrentObservationTime >= ObservationTime)
    {
        CompleteObservation();
    }
}

void AQuest_CrowdObservationSystem::CompleteObservation()
{
    bQuestCompleted = true;
    bIsObserving = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdObservationSystem: Observação completada com sucesso!"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("MISSÃO COMPLETADA: Observação de comportamento de dinossauros concluída!"));
    }
    
    // Executar evento de conclusão
    OnQuestCompleted.ExecuteIfBound();
}

TArray<AActor*> AQuest_CrowdObservationSystem::FindNearbyDinosaurs()
{
    TArray<AActor*> NearbyDinosaurs;
    
    if (!GetWorld())
        return NearbyDinosaurs;
    
    // Procurar por todos os actores na área
    TArray<AActor*> OverlappingActors;
    DetectionSphere->GetOverlappingActors(OverlappingActors);
    
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && IsDinosaurActor(Actor))
        {
            NearbyDinosaurs.Add(Actor);
        }
    }
    
    return NearbyDinosaurs;
}

bool AQuest_CrowdObservationSystem::IsPlayerCharacter(AActor* Actor)
{
    if (!Actor)
        return false;
    
    // Verificar se é o character do jogador
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn() == Actor)
    {
        return true;
    }
    
    return false;
}

bool AQuest_CrowdObservationSystem::IsDinosaurActor(AActor* Actor)
{
    if (!Actor)
        return false;
    
    FString ActorName = Actor->GetName();
    FString ActorClass = Actor->GetClass()->GetName();
    
    // Verificar nomes que indicam dinossauros
    if (ActorName.Contains(TEXT("Dinosaur")) || 
        ActorName.Contains(TEXT("TRex")) || 
        ActorName.Contains(TEXT("Raptor")) || 
        ActorName.Contains(TEXT("Brachio")) ||
        ActorClass.Contains(TEXT("Dinosaur")))
    {
        return true;
    }
    
    return false;
}

void AQuest_CrowdObservationSystem::SetObservationParameters(float NewRadius, int32 NewMinCrowdSize, float NewObservationTime)
{
    ObservationRadius = NewRadius;
    MinCrowdSize = NewMinCrowdSize;
    ObservationTime = NewObservationTime;
    
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(ObservationRadius);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdObservationSystem: Parâmetros atualizados - Raio: %.0f, MinCrowd: %d, Tempo: %.0f"), 
           ObservationRadius, MinCrowdSize, ObservationTime);
}

void AQuest_CrowdObservationSystem::ResetQuest()
{
    bQuestCompleted = false;
    bIsObserving = false;
    CurrentObservationTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdObservationSystem: Quest reiniciada"));
}